/*-
 * Copyright (c) 1995-2000 Steffen Seeger
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
 * KGI ISO font renderer
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_ATOMIC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>
#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_kgirndr.h>
#include <dev/kgc/kgc_isorndr.h>

#include "render_if.h"

typedef struct {
	/*
	 * Must be there if you use KGI resources interface
	 */
	__KGC_KGIRNDR_DATA
	kgi_console_font_t	*font;
} isorndr_meta;

static kgi_u_t 
isorndr_ctop(render_t r, kgi_isochar_t sym)
{
	isorndr_meta *isorndr;
	kgi_console_font_t *font;
	kgi_u_t row;
	kgi_u_t cell;
	kgi_console_font_cellinfo_t *cellinfo;
	kgi_u_t l = 0, h, m;

	isorndr = kgc_render_meta(r);
	font = isorndr->font;
	row  = KGI_ISOCHAR_ROW(sym);
	cell = KGI_ISOCHAR_CELL(sym);

	KGI_ASSERT(font);
	cellinfo = font->info->cellinfo[row];

	if (row == 0xF0) /* Display directly. */
		return (cell);

	if ((sym & 0xFFFF0000) || cellinfo == NULL) 
		return (font->info->default_pos);

	if (test_bit(row, font->info->map_direct)) 
		return (((kgi_u8_t *) cellinfo) [cell]);

	if (sym > cellinfo->sym)
		return (font->info->default_pos); /* Symbol not in list? */

	h = cellinfo->pos;
	cellinfo++;

	if (sym < cellinfo->sym) 
		return (font->info->default_pos);

	if (cellinfo->sym == sym)
		return (cellinfo->pos);

	do { /* Search matching cell. */
		if (cellinfo[h].sym == sym) 
			return (cellinfo[h].pos);

		m = (--h + l) >> 1;

		if (sym > cellinfo[m].sym) 
			l = m;
		else  
			h = m;
	} while (l < h); 

	return (font->info->default_pos); /* No position defined. */
}

static kgi_isochar_t 
isorndr_ptoc(render_t r, kgi_u_t pos)
{
	isorndr_meta *isorndr;
	kgi_console_font_t *font;

	isorndr = kgc_render_meta(r);
	font = isorndr->font;

	return (font->info->inversemap[(pos < font->info->positions) ? pos : 0]);
}

static void 
isorndr_putcs(render_t r, kgc_textbuf_t *tb, kgi_u_t start,
			  kgi_u_t offset, kgi_u_t count)
{
	isorndr_meta *render;

	render = kgc_render_meta(r);

	if (render->text16) {
		(render->text16->PutText16)(render->text16, offset, 
				tb->buf + start, count);
	}
}

static void 
isorndr_get_sizes(render_t r, kgi_ucoord_t *size, kgi_ucoord_t *virt)
{
	isorndr_meta *render;

	render = kgc_render_meta(r);

	size->x = render->text16->size.x;
	size->y = render->text16->size.y;
	virt->x = render->text16->size.x;
	virt->y = render->text16->size.y;
}

static void 
isorndr_set_sizes(render_t r, kgi_ucoord_t *size, kgi_ucoord_t *virt,
		  kgi_u8_t depth)
{
	isorndr_meta *render;

	render = kgc_render_meta(r);

	/* Only virtual sizes may be modified. */
	render->text16->size.x = virt->x;
	render->text16->size.y = virt->y;
}

/*
 * console font handling
 */
static kgi_console_font_t *
isorndr_default_font(render_t r)
{
	isorndr_meta *render;
	kgi_u_t i;
 
	render = kgc_render_meta(r);

	KGI_DEBUG(3, "searching font for height %i", render->text16->font.y);
	for (i = 0; default_font[i] && 
		(default_font[i]->size.y > render->text16->font.y); i++) 
		;
	KGI_DEBUG(3, "found font %i (%p)", i, default_font[i]);
	return (default_font[i]);
}

static kgi_s_t
isorndr_init(render_t r)
{
	isorndr_meta *render;

	render = kgc_render_meta(r);
	/* Common initialization, especially concerning resources */
	kgirndr_init(r);
	
	render->font = isorndr_default_font(r);

	return (KGI_EOK);
}

static render_method_t isorndr_methods[] = {
	/* Specific methods */
	RENDERMETHOD(render_init,		isorndr_init),
	RENDERMETHOD(render_put_text,		isorndr_putcs),
	RENDERMETHOD(render_ptoc,		isorndr_ptoc),
	RENDERMETHOD(render_ctop,		isorndr_ctop),
	RENDERMETHOD(render_get_sizes,		isorndr_get_sizes),
	RENDERMETHOD(render_set_sizes,		isorndr_set_sizes),
	/* Methods using KGI generic interface */
	RENDERMETHOD(render_map,		kgirndr_map),
	RENDERMETHOD(render_ptoa,		kgirndr_ptoa_color),
	RENDERMETHOD(render_atop,		kgirndr_atop_color),
	RENDERMETHOD(render_show_gadgets,	kgirndr_show_gadgets),
	RENDERMETHOD(render_hide_gadgets,	kgirndr_hide_gadgets),
	RENDERMETHOD(render_undo_gadgets,	kgirndr_undo_gadgets),
	RENDERMETHOD_END
};

render_driver_t isorndr_driver = {
	"isorndr",
	isorndr_methods,
	sizeof(isorndr_meta)
};

DECLARE_RENDER(isorndr, isorndr_driver, isorndr_meta);

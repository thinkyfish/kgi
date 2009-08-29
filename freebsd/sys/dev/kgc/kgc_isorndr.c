/*-
 * Copyright (C) 1995-2000 Steffen Seeger
 * Copyright (C) 2003 Nicholas Souchu
 *
 * This file is distributed under the terms and conditions of the 
 * MIT/X public license. Please see the file COPYRIGHT.MIT included
 * with this software for details of these terms and conditions.
 * Alternatively you may distribute this file under the terms and
 * conditions of the GNU General Public License. Please see the file 
 * COPYRIGHT.GPL included with this software for details of these terms
 * and conditions.
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
	isorndr_meta *isorndr = kgc_render_meta(r);
	kgi_console_font_t *font = isorndr->font;
	kgi_u_t row  = KGI_ISOCHAR_ROW(sym);
	kgi_u_t cell = KGI_ISOCHAR_CELL(sym);
	kgi_console_font_cellinfo_t *cellinfo;
	kgi_u_t l = 0, h, m;

	KRN_ASSERT(font);
	cellinfo = font->info->cellinfo[row];

	if (row == 0xF0)		/* display directly */
		return (cell);

	if ((sym & 0xFFFF0000) || !cellinfo) 
		return (font->info->default_pos);

	if (test_bit(row, font->info->map_direct)) 
		return (((kgi_u8_t *) cellinfo) [cell]);

	if (sym > cellinfo->sym)
		return (font->info->default_pos); /* symbol not in list?	*/

	h = cellinfo->pos;
	cellinfo++;

	if (sym < cellinfo->sym) 
		return (font->info->default_pos);

	if (cellinfo->sym == sym)
		return (cellinfo->pos);

	do {	/* search matching cell */
		if (cellinfo[h].sym == sym) 
			return (cellinfo[h].pos);

		m = (--h + l) >> 1;

		if (sym > cellinfo[m].sym) {
			l = m;
		} else { 
			h = m;
		}
	} while (l < h); 

	return (font->info->default_pos); /* no position defined */
}

static kgi_isochar_t 
isorndr_ptoc(render_t r, kgi_u_t pos)
{
	isorndr_meta *isorndr = kgc_render_meta(r);
	kgi_console_font_t *font = isorndr->font;

	return (font->info->inversemap[(pos < font->info->positions) ? pos : 0]);
}

static void 
isorndr_putcs(render_t r, kgc_textbuf_t *tb, kgi_u_t start,
			  kgi_u_t offset, kgi_u_t count)
{
	isorndr_meta *render = kgc_render_meta(r);

	if (render->text16) {
		(render->text16->PutText16)(render->text16, offset, tb->buf + start,
		 count);
	}
}

static void 
isorndr_get_sizes(render_t r, kgi_ucoord_t *size, kgi_ucoord_t *virt)
{
	isorndr_meta *render = kgc_render_meta(r);

	size->x = render->text16->size.x;
	size->y = render->text16->size.y;
	virt->x = render->text16->size.x;
	virt->y = render->text16->size.y;
}

static void 
isorndr_set_sizes(render_t r, kgi_ucoord_t *size, kgi_ucoord_t *virt,
			      kgi_u8_t depth)
{
	isorndr_meta *render = kgc_render_meta(r);

	/* only virtual sizes may be modified */
	render->text16->size.x = virt->x;
	render->text16->size.y = virt->y;
}

/*
 * console font handling
 */
static kgi_console_font_t *
isorndr_default_font(render_t r)
{
	isorndr_meta *render = kgc_render_meta(r);
	kgi_u_t i = 0;

	KRN_DEBUG(3, "searching font for height %i", render->text16->font.y);
	for (i = 0; default_font[i] && 
		(default_font[i]->size.y > render->text16->font.y); i++) 
		;
	KRN_DEBUG(3, "found font %i (%p)", i, default_font[i]);
	return (default_font[i]);
}

static kgi_s_t
isorndr_init(render_t r)
{
	isorndr_meta *render = kgc_render_meta(r);

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

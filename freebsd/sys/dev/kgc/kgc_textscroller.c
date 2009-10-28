/*
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
 * KGI text scroller implementation.
 */
 
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_ATOMIC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_scroller.h>
#include <dev/kgc/kgc_render.h>

#include "scroller_if.h"
#include "render_if.h"

#define	MAX_COLUMNS	160
#define	MAX_TAB_STOPS	MAX_COLUMNS/(8*sizeof(unsigned long))

typedef struct {
	/* the scroller textbuffer */
	kgc_textbuf_t	tb;	/* interface with the renders */
	/* miscellaneous scroller variables */
	kgi_u_t		from, to;
	kgi_u_t		offset;
	kgi_u_t		last_offset;
	kgi_u_t		top, bottom;
	kgi_u_t		origin;
	kgi_u_t		org;
	kgi_u_t		x,y,s_x,s_y;
	kgi_u_t		wpos;
	kgi_u_t		pos;
	kgi_u_t		mark;
	unsigned long	tab_stop[MAX_TAB_STOPS];
	/* attributes */
	kgi_u_t		attrfl,s_attrfl;/* global (default) kgi attribute */
	kgi_u_t		attr;		/* global (default) pixel attribute */
	kgi_u_t		erase;		/* erase pixel + attribute */
} textscroller_meta;

static void 
textscroller_mark(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);	
	scroll->mark = scroll->wpos;
}

static void 
textscroller_modified(scroller_t s, kgi_u_t from, kgi_u_t to)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	KRN_ASSERT(from <= to);

	if (from < scroll->from) 
		scroll->from = from;

	if (scroll->to < to) 
		scroll->to = to;
}

static void 
textscroller_modified_mark(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	if (scroll->mark != scroll->wpos)
		textscroller_modified(s, scroll->mark, scroll->wpos);
}

static void 
textscroller_modified_wrap(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	textscroller_modified(s, scroll->mark, scroll->wpos + 1);
}

static void 
textscroller_update_attr(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	scroll->erase = scroll->attr = RENDER_ATOP(cons->render, scroll->attrfl);
	scroll->erase |= RENDER_CTOP(cons->render, 0x20);
}

static void
textscroller_cr(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	RENDER_UNDO_GADGETS(cons->render);

	scroll->wpos -= scroll->x;
	scroll->pos -= scroll->x;
	scroll->x = 0;

	RENDER_SHOW_GADGETS(cons->render, scroll->x, scroll->y, scroll->offset);

	cons->flags &= ~KGI_CF_NEED_WRAP;
}

static void 
textscroller_write(scroller_t s, kgi_isochar_t c)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	scroll->tb.buf[scroll->pos] = scroll->attr | RENDER_CTOP(cons->render, c);

	if (scroll->x == (scroll->tb.size.x - 1)) {
		cons->flags |= KGI_CF_NEED_WRAP;
		return;
	}

	scroll->x++;
	scroll->wpos++;
	scroll->pos++;
}

/*
 * textscroller_sync() has to sync the display with the scroller state. The
 * scroller keeps track of the area modified. If there were any 
 * modifications since the last textscroller_sync(), the area from cons->from
 * cons->to contained in the currently displayed area needs to be updated.
 */
static void 
textscroller_sync(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_s_t orig;
	kgi_u_t render_flags;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	KRN_ASSERT(cons);

	RENDER_GET(cons->render, 0, 0, &render_flags);
	if (!(render_flags & KGI_RF_NEEDS_UPDATE))
		return;

	orig = scroll->origin - scroll->offset;
	if (orig < 0) {
		if ((cons->flags & KGI_CF_NO_HARDSCROLL) ||
			(!(cons->flags & KGI_CF_NO_HARDSCROLL) &&
			(cons->flags & KGI_CF_SPLITLINE))) {
				orig += scroll->tb.virt.y;
		} else 
			orig = 0;
	}
	KRN_ASSERT((kgi_u_t) orig < scroll->tb.virt.y);

	if (cons->flags & KGI_CF_NO_HARDSCROLL) {
		kgi_s_t frm = scroll->from;
		kgi_s_t end = scroll->to;
		kgi_s_t tb_total = scroll->tb.total;

		kgi_s_t back = (orig *= scroll->tb.virt.x) + scroll->tb.frame;
		kgi_u16_t *buf = scroll->tb.buf;

		KRN_ASSERT(buf);

		if (scroll->offset || (scroll->last_offset != scroll->offset) ||
			(cons->flags & KGI_CF_SCROLLED)) {
			frm = orig;
			end = back;
		}
		cons->flags &= ~KGI_CF_SCROLLED;

		if ((end < frm) || (end < orig) || (back < frm))
			return;

		if (frm < orig) 
			frm = orig;

		if (back < end) 
			end = back;

		RENDER_UNDO_GADGETS(cons->render);

		if (frm < tb_total) {
			if (tb_total >= end) {
				RENDER_PUT_TEXT(cons->render, &scroll->tb, frm,
						frm-orig, end-frm);
			} else {
				RENDER_PUT_TEXT(cons->render, &scroll->tb, frm,
						frm-orig, tb_total-frm);

				RENDER_PUT_TEXT(cons->render, &scroll->tb, 0,
						tb_total-orig, end-tb_total);
			}
		} else {
			RENDER_PUT_TEXT(cons->render, &scroll->tb, frm-tb_total,
					frm-orig, end-frm);
		}

		RENDER_SHOW_GADGETS(cons->render, scroll->x, scroll->y,
				    scroll->offset);
	} else {
/*
		if (cons->flags & KGI_CF_SCROLLED) {

			cons->flags &= ~KGI_CF_SCROLLED;
			kgidev_set_origin(&(cons->kgi), 0, 0, 
				orig*cons->font->size.y);

			if (cons->flags & KGI_CF_SPLITLINE) {

				kgi_s_t split = cons->font->size.y *
					((orig+scroll->tb.size.y < scroll->tb.virt.y)
						? cons->text16.size.y
						: scroll->tb.virt.y - orig);

				kgidev_set_split(&(cons->kgi), split - 1);
			}
		}
*/
	}

	scroll->last_offset = scroll->offset;
	scroll->from = (kgi_u_t) - 1L;
	scroll->to   = 0;
}

/*
 * cons->tb.buf may be set if allocated statically before calling
 * textscroller_init()
 */
static kgi_s_t 
textscroller_init(scroller_t s, kgi_u16_t *buffer)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_ucoord_t render_size, render_virt;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	/* The buffer address may be passed in case of static allocation */
	if (buffer) {
		scroll->tb.buf = buffer;
	} else {
		if (!(scroll->tb.buf = kgi_kmalloc(CONFIG_KGII_CONSOLEBUFSIZE))) {
			KRN_ERROR("text buffer allocation failed");
			return (KGI_ENOMEM);
		}
		memset(scroll->tb.buf, 0, CONFIG_KGII_CONSOLEBUFSIZE);
		scroll->tb.flags |= TBF_ALLOCATED;
	}

	RENDER_GET(cons->render, &render_size, &render_virt, 0);

	scroll->tb.size.x = render_size.x;
	scroll->tb.size.y = render_size.y;

	scroll->tb.virt.x = render_size.x;
	scroll->tb.virt.y = CONFIG_KGII_CONSOLEBUFSIZE / 
		(sizeof(*scroll->tb.buf)*scroll->tb.virt.x);
	if (scroll->tb.virt.y < 2 * render_size.y) {
		if ((scroll->tb.buf) && (scroll->tb.flags & TBF_ALLOCATED)) {
			KRN_ERROR("text buffer too small, reset failed");
			kgi_kfree(scroll->tb.buf);
			scroll->tb.buf = NULL;
		}
		return (KGI_ENOMEM);
	}

	if (scroll->tb.virt.y < render_size.y) 
		render_virt.y = scroll->tb.virt.y;

	if (scroll->tb.virt.y > render_size.y) 
		render_virt.y = scroll->tb.size.y;


	RENDER_SET(cons->render, &render_size, &render_virt);

	scroll->tb.frame = scroll->tb.virt.x * scroll->tb.size.y;
	scroll->tb.total = scroll->tb.virt.x * scroll->tb.virt.y;

	scroll->from = scroll->tb.total;
	scroll->to = 0;
	scroll->offset = scroll->last_offset = 0;

	cons->flags |= KGI_CF_NO_HARDSCROLL;

	CONSOLE_SET_MODE(cons, KGI_CM_SHOW_CURSOR);

	KRN_ASSERT(render_size.x == scroll->tb.size.x);
	KRN_ASSERT(render_size.y == scroll->tb.size.y);
	KRN_ASSERT(render_virt.x == scroll->tb.virt.x);

	KRN_DEBUG(4, "%ix%i (%ix%i virt) scroller initialized",
		render_size.x, render_size.y,
		render_virt.x, render_virt.y);

	return (0);
}

#if 0
static kgi_s_t textscroller_resize(scroller_t s, kgi_u_t new_sizex,
	kgi_u_t new_sizey)
{
	textscroller_meta *scroll = kgc_scroller_meta(s);
	kgi_u16_t *src, *dst;
	kgi_u_t new_tb_sizey, i,j;

	KRN_ASSERT(new_sizex);
	KRN_ASSERT(new_sizey);

	new_tb_sizey = CONFIG_KGII_CONSOLEBUFSIZE /
		(sizeof(*scroll->tb.buf)*new_sizex);

	if (new_tb_sizey < 2*new_sizey) {

		KRN_DEBUG(1, "text buffer to small, resize failed")
	}

#warning finish!
}
#endif

static void 
textscroller_bs(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	RENDER_UNDO_GADGETS(cons->render);

	if (scroll->x) {
		scroll->x--;
		scroll->wpos--;
		scroll->pos--;
		cons->flags &= ~KGI_CF_NEED_WRAP;
	}

	RENDER_SHOW_GADGETS(cons->render, scroll->x, scroll->y, scroll->offset);
}

static void 
textscroller_hts(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	scroll->tab_stop[scroll->x >> 5] |= (1 << (scroll->x & 31));
}

static void 
textscroller_tbc(scroller_t s, kgi_u_t tab)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	if (tab == 0) {		
		scroll->tab_stop[scroll->x >> 5] &= ~(1 << (scroll->x & 31));		
	} else {
		if (tab == 3) {			
			scroll->tab_stop[0] =
			scroll->tab_stop[1] =
			scroll->tab_stop[2] =
			scroll->tab_stop[3] =
			scroll->tab_stop[4] = 0;
		}
	}
}

static void 
textscroller_ht(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	RENDER_UNDO_GADGETS(cons->render);

	scroll->pos -= scroll->x;
	scroll->wpos -= scroll->x;
	while (scroll->x < scroll->tb.size.x - 1) {		
		scroll->x++;
		if (scroll->tab_stop[scroll->x >> 5] &
		    (1 << (scroll->x & 31))) {			
			break;
		}
	}
	scroll->pos  += scroll->x;
	scroll->wpos += scroll->x;

	RENDER_SHOW_GADGETS(cons->render, scroll->x, scroll->y, scroll->offset);
}

static void 
textscroller_mksound(scroller_t s, kgi_u_t pitch, kgi_u_t duration)
{

	KRN_DEBUG(3, "textscroller_mksound(%p, %i, %i) not implemented yet.",
		  s, pitch, duration);
}

/*
 * Scroll down n lines between t(op) and b(ottom) and clear the new top
 * area using ERASE. If n is greater than b - t, we scroll only b - t lines.
 * If bottom is more than SIZE_Y, bottom is less or equal top or n is zero
 * we do nothing.
 */
static void 
textscroller_down(scroller_t s, kgi_u_t t, kgi_u_t b, kgi_u_t n)
{
	textscroller_meta *scroll;
	kgi_u_t src, dst, end;	/* first/last pixel to move	*/
	kgi_u_t cnt;		/* size of area to clean	*/
	kgi_u_t h1, h2;		/* subexpression		*/
	kgi_u_t tb_total;	/* text buffer size		*/
	kgc_textbuf_t *tb;

	scroll = kgc_scroller_meta(s);
	tb = &scroll->tb;

	if ((scroll->tb.size.y < b) || (b <= t) || n == 0) 
		return;

	if (n > (b - t)) 
		n = b - t;

	end  = t + scroll->origin;
	dst  = b + scroll->origin;
	tb_total = scroll->tb.virt.x;
	end *= tb_total;
	dst *= tb_total;
	cnt  = tb_total*n;
	tb_total *= scroll->tb.virt.y;
	src  = dst - cnt;

	KRN_ASSERT((end <= src) && (src < dst));

	textscroller_modified(s, end, dst);

	if (src > end) {
		if (end >= tb_total) {
			dst -= tb_total;
			src -= tb_total;
			end -= tb_total;
		}

		if (dst <= tb_total) {
			tb_move(tb, end + cnt, end, src - end);
		} else {
			if (src > tb_total) {
				h1 = src - tb_total;
				tb_move(tb, dst - src, 0, h1);
				src -= h1;
				dst -= h1;
			}

			h1 = dst - tb_total;
			h2 = src - end;

			if (h1 < h2) {
				tb_move(tb, 0, src - h1, h1);
				h2 -= h1;
				tb_move(tb, tb_total - h2, end, h2);
			} else 
				tb_move(tb, dst - h2 - tb_total, end, h2);
		}
	}

	/* clear top area */

	if (end < tb_total) {
		if (tb_total >= (end + cnt)) {
			tb_set(tb, end, scroll->erase, cnt);

		} else {
			tb_set(tb, end, scroll->erase, tb_total - end);
			tb_set(tb, 0, scroll->erase, end + cnt - tb_total);
		}
	} else {
		tb_set(tb, end - tb_total, scroll->erase, cnt);
	}
}

static void 
textscroller_scroll_top(scroller_t s, kgi_u_t n)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	textscroller_down(s, scroll->y, scroll->bottom, n);
}

/*
 * erase in line using ERASE. The area erased depends on arg. 
 */
static void 
textscroller_erase_line(scroller_t s, kgi_u_t arg)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t src, cnt;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	switch (arg) {
	case 0:	/* from cursor to end of line	*/
		src = scroll->wpos;
		cnt = scroll->tb.virt.x - scroll->x;
		break;
	case 1:	/* from line start to cursor	*/
		src = scroll->wpos - scroll->x;
		cnt = scroll->x + 1;
		break;
	case 2:	/* erase entire line	*/
		src = scroll->wpos - scroll->x;
		cnt = scroll->tb.virt.x;
		break;
	default:
		return;
	}

	textscroller_modified(s, src, src + cnt);

	if (scroll->tb.total <= src) 
		src -= scroll->tb.total;

	tb_set(&scroll->tb, src, scroll->erase, cnt);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}

/* 
 * Insert n ERASE characters. 
 */
static void 
textscroller_insert_chars(scroller_t s, kgi_u_t n)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t src;
	kgi_u_t cnt;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
	src = scroll->wpos;
	cnt = scroll->tb.virt.x - scroll->x;

	KRN_ASSERT((cnt > 0) && (n > 0));

	textscroller_modified(s, src, src + cnt);

	if (scroll->tb.total <= src) 
		src -= scroll->tb.total;

	if (n < cnt) {
		tb_move(&scroll->tb, src + n, src, cnt - n);
		cnt = n;
	}

	tb_set(&scroll->tb, src, scroll->erase, cnt);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/* 
 * Delete n characters. Clear the end of the line using ERASE. 
 */
static void 
textscroller_delete_chars(scroller_t s, kgi_u_t n)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t cnt;
	kgi_u_t src;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
	cnt = scroll->tb.virt.x - scroll->x;
	src = scroll->wpos;

	textscroller_modified(s, src, src + cnt);

	if (scroll->tb.total <= src) 
		src -= scroll->tb.total;

	if (n < cnt) {
		tb_move(&scroll->tb, src, src + n, cnt - n);
		tb_set(&scroll->tb, src + cnt - n, scroll->erase, n);
	} else 
		tb_set(&scroll->tb, src, scroll->erase, cnt);

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/* 
 * Erase the following n chars. (maximum is end of line). 
 */
static void 
textscroller_erase_chars(scroller_t s, kgi_u_t n)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	KRN_ASSERT(n > 0);
	if (n > (scroll->tb.size.x - scroll->x)) 
		n = scroll->tb.virt.x - scroll->x;

	tb_set(&scroll->tb, scroll->pos, scroll->erase, n);
	textscroller_modified(s, scroll->wpos, scroll->wpos + n);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}

/*
 * Do a reverse linefeed. This means to scroll down one line if the cursor
 * is at the top of the scrolling region or simply to move the cursor up
 * one line.
 */
static void 
textscroller_reverse_lf(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	RENDER_UNDO_GADGETS(cons->render);

	if (scroll->y == scroll->top) 
		textscroller_down(s, scroll->top, scroll->bottom, 1);
	else {
		if (scroll->y > scroll->top) {
			scroll->y--;
			scroll->wpos -= scroll->tb.virt.x;

			if (scroll->pos < scroll->tb.virt.x) {
				scroll->pos += 
				scroll->tb.total - scroll->tb.virt.x;
			} else 
				scroll->pos -= scroll->tb.virt.x;
		}
	}

	RENDER_SHOW_GADGETS(cons->render, scroll->x, scroll->y, scroll->offset);

	cons->flags &= ~KGI_CF_NEED_WRAP;
}

static void 
textscroller_done(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	if ((scroll->tb.buf) && (scroll->tb.flags & TBF_ALLOCATED)) {
		kgi_kfree(scroll->tb.buf);
		scroll->tb.buf = NULL;
	}
}

/* 
 * Erase in display using ERASE. The area cleared depends on arg.
 */
static void 
textscroller_erase_display(scroller_t s, kgi_u_t arg)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t src, cnt;
	kgi_u_t tb_total;

 	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
	cnt = scroll->tb.frame;	/* area to erase	*/
	tb_total = scroll->tb.virt.x*scroll->tb.virt.y;

	switch (arg) {
	case 0:	/* from cursor to end of screen	*/
		src = scroll->wpos;
		cnt -= scroll->wpos - scroll->org;
		break;
	case 1:	/* from start of screen to cursor */
		src = scroll->org;
		cnt = scroll->wpos + 1 - scroll->org;
		break;
	case 2:	/* erase entire screen */
		src = scroll->org;
		break;
	default:
		return;
	}

	textscroller_modified(s, src, src + cnt);

	if (src < tb_total) {
		if (tb_total >= (src + cnt)) 
			tb_set(&scroll->tb, src, scroll->erase, cnt);
		else {
			tb_set(&scroll->tb, src, scroll->erase, tb_total-src);
			tb_set(&scroll->tb, 0, scroll->erase, 
				src + cnt - tb_total);
		}
	} else 
		tb_set(&scroll->tb, src - tb_total, scroll->erase, cnt);

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/* 
 * Move cursor to the specified position. We have to check all boundaries.
 */
static void 
textscroller_gotoxy(scroller_t s, kgi_s_t new_x, kgi_s_t new_y)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_s_t max_y, min_y;
	kgi_s_t max_x;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
#define	min_x	0

	RENDER_UNDO_GADGETS(cons->render);

	if (CONSOLE_MODE(cons, KGI_CM_ORIGIN)) {
		min_y = scroll->top;
		max_y = scroll->bottom;
	} else {
		min_y = 0;
		max_y = scroll->tb.size.y;
	}

	max_x = scroll->tb.size.x;
	scroll->x = 
		(new_x < min_x) ? min_x : ((new_x < max_x) ? new_x : max_x-1);
	scroll->y = 
		(new_y < min_y) ? min_y : ((new_y < max_y) ? new_y : max_y-1);

	scroll->pos = scroll->wpos = scroll->org +
		scroll->y*scroll->tb.virt.x + scroll->x;

	if (scroll->tb.total <= scroll->pos) 
		scroll->pos -= scroll->tb.total;

	RENDER_SHOW_GADGETS(cons->render, scroll->x, scroll->y, scroll->offset);

	cons->flags &= ~KGI_CF_NEED_WRAP;
}

static void 
textscroller_gotox(scroller_t s, kgi_s_t x)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	textscroller_gotoxy(s, x, scroll->y);
}

static void 
textscroller_gotoy(scroller_t s, kgi_s_t y)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	textscroller_gotoxy(s, scroll->x, y);
}

static void 
textscroller_move(scroller_t s, kgi_s_t col, kgi_s_t lin)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	textscroller_gotoxy(s, scroll->x + col, scroll->y + lin);
}

static void 
textscroller_need_sync(scroller_t s)
{
	/* XXX mark_bh(CONSOLE_BH); */
	return;
}

/*
 * Increase the scroll-back offset by <lines> lines. <lines> defaults to
 * half the number of lines of the visible screen if less or equal zero. 
 * As we may be called from a IRQ service, we do not sync the display
 * because this may take some time. We just mark the CONSOLE_BH and
 * do syncing there.
 */
static void 
textscroller_backward(scroller_t s, int lines)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t max;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
	max = scroll->tb.virt.y - scroll->tb.size.y;

	if (lines <= 0) 
		lines = scroll->tb.size.y / 2;

	scroll->offset += lines;
	if (scroll->offset > max) 
		scroll->offset = max;

	CONSOLE_CLEAR_MODE(cons, KGI_CM_SHOW_CURSOR);

	cons->flags |= KGI_CF_SCROLLED;

	textscroller_need_sync(s);
}

/*
 * Decrease the scroll-back offset by <lines> lines. <lines> defaults to
 * half the number of lines of the visible screen if less or equal zero.
 * As we may be called from an IRQ service, we do not sync the display
 * because this may take some time. We just mark the CONSOLE_BH
 * and do syncing there.
 */
static void 
textscroller_forward(scroller_t s, int lines)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	if (lines <= 0) 
		lines = scroll->tb.size.y / 2;

	scroll->offset -= lines;
	if (scroll->offset >= scroll->tb.total) 
		scroll->offset = 0;

	if (scroll->offset == 0) 
		CONSOLE_SET_MODE(cons, KGI_CM_SHOW_CURSOR);

	cons->flags |= KGI_CF_SCROLLED;

	textscroller_need_sync(s);
}

/*
 * Scroll up n lines between t(op) and b(ottom) and erase the 'new' bottom
 * area using ERASE. If n is greater than b - t, we scroll only b - t lines.
 * If bottom is more than SIZE_Y, bottom is less or equal top, or n is zero
 * we do nothing.
 */
static void 
textscroller_up(scroller_t s, kgi_u_t t, kgi_u_t b, kgi_u_t n)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t src, end;	/* first and last pixel to move	*/
	kgi_u_t clean, cnt;	/* first pixel/# pixels to clean*/ 
	kgi_u_t dst;		/* pixel to move to	 */
	kgi_u_t h1, h2;		/* temporary variables */
	kgi_u_t tb_total;
	kgc_textbuf_t *tb;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
	tb = &scroll->tb;

	if ((scroll->tb.size.y < b) || (b <= t) || n == 0) 
		return;

	if (n > (b - t)) 
		n = b - t;

	dst = t + scroll->origin;
	end = b + scroll->origin;

	tb_total = scroll->tb.virt.x;

	dst *= tb_total;
	end *= tb_total;

	cnt = n * tb_total;
	tb_total *= scroll->tb.virt.y;
	src = dst + cnt;

	KRN_ASSERT((dst < src) && (src <= end));

	if (CONSOLE_MODE(cons, KGI_CM_ALT_SCREEN) || t 
		|| (b < scroll->tb.size.y)) {
			/* can't use origin adjustment */
			textscroller_modified(s, dst, end);

			if (end <= src) 
				clean = dst;
			else {
				clean = end - cnt;

				if (dst >= tb_total) {
					dst -= tb_total;
					src -= tb_total;
					end -= tb_total;
				}

				KRN_ASSERT(dst < tb_total);

				if (end <= tb_total) 
					tb_move(tb, dst, src, end - src);
				else {
					if (src < tb_total) {
						h1 = tb_total - src;
						tb_move(tb, dst, src, h1);
						src += h1;
						dst += h1;
					}

					h1 = tb_total - dst;
					h2 = end - src;

					KRN_ASSERT((h1 > 0) && (h2 > 0));

					if (h1 < h2) {
						tb_move(tb, dst, 
							src - tb_total, h1);
						tb_move(tb, 0,
							 src - dst, h2 - h1);
					} else {
						tb_move(tb, dst,
							 src - tb_total, h2);
					}
				}
			}
		} else {
			/* adjust origin */
			cons->flags |= KGI_CF_SCROLLED;

			if (scroll->offset) {
				scroll->offset += n;
				if (scroll->offset > (scroll->tb.virt.y 
						- scroll->tb.size.y)) {
					scroll->offset = 0;
				}
			}

			scroll->origin += n;
			scroll->org    += cnt;
			scroll->wpos   += cnt;
			scroll->pos    += cnt;

			if (tb_total <= scroll->pos) 
				scroll->pos -= tb_total;

			if (tb_total <= scroll->org) {
				scroll->org 	-= tb_total;
				scroll->wpos	-= tb_total;
				scroll->origin	-= scroll->tb.virt.y;
			}
			clean = scroll->org + scroll->tb.frame - cnt;
		}

		/* clear bottom area */
		textscroller_modified(s, clean, clean + cnt);

		if (clean < tb_total) {
			if (tb_total >= (clean + cnt)) 
				tb_set(tb, clean, scroll->erase, cnt);
			else {
				tb_set(tb, clean, 
					scroll->erase, tb_total - clean);
				tb_set(tb, 0, 
					scroll->erase, clean + cnt - tb_total);
			}
		} else 
			tb_set(tb, clean - tb_total, scroll->erase, cnt);
}

static void 
textscroller_scroll_bottom(scroller_t s, kgi_u_t n)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	textscroller_up(s, scroll->y, scroll->bottom, n);
}

/*
 * Do a linefeed. This means to scroll up one line if the cursor is at
 * the bottom line of the scrolling region or simply to move the cursor
 * down one line.
 */
static void 
textscroller_lf(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	if ((scroll->y + 1) == scroll->bottom)
		textscroller_up(s, scroll->top, scroll->bottom, 1);
	else {
		if (scroll->y < (scroll->tb.size.y - 1)) {
			scroll->y++;
			scroll->wpos += scroll->tb.virt.x;
			scroll->pos  += scroll->tb.virt.x;

			if (scroll->tb.total <= scroll->pos) 
				scroll->pos -= scroll->tb.total;
		}
	}

	cons->flags &= ~KGI_CF_NEED_WRAP;
}

static void 
textscroller_reset(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);

	CONSOLE_SET_MODE(cons, KGI_CM_SHOW_CURSOR);
	CONSOLE_SET_MODE(cons, KGI_CM_AUTO_WRAP);

	scroll->tab_stop[0] = 0x01010100;
	scroll->tab_stop[1] = scroll->tab_stop[2] = scroll->tab_stop[3] =
		scroll->tab_stop[4] = 0x01010101;

	scroll->origin = scroll->org = scroll->top = 0;
	scroll->bottom = scroll->tb.size.y;

	scroll->attrfl = KGI_CA_NORMAL | 
		KGI_CA_COLOR(KGI_CC_LIGHTGRAY, KGI_CC_BLACK);
}

static void 
textscroller_get(scroller_t s, kgi_ucoord_t *size, kgi_u_t *top,
		kgi_u_t *bottom, kgi_u_t *x, kgi_u_t *y, kgi_u_t *attrfl,
		kgi_u_t *erase)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	if (size) {
		size->x = scroll->tb.size.x;
		size->y = scroll->tb.size.y;
	}

	if (top && bottom) {
		*top = scroll->top;
		*bottom = scroll->bottom;
	}

	if (x && y) {
		*x = scroll->x;
		*y = scroll->y;
	}

	if (attrfl) 
		*attrfl = scroll->attrfl;

	if (erase) 
		*erase = scroll->erase;
}

static void 
textscroller_set(scroller_t s, kgi_u_t attrfl, kgi_u_t erase)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	scroll->attrfl = attrfl;
	scroll->erase = erase;
}

static void 
textscroller_margins(scroller_t s, kgi_u_t top, kgi_u_t bottom)
{
	textscroller_meta *scroll = kgc_scroller_meta(s);

	if (top == 0) 	
		top++;

	if (bottom == 0) 		
		bottom = scroll->tb.size.y;
	
	/* Minimum allowed region is 2 lines */
	if ((top < bottom) && (bottom <= scroll->tb.size.y)) {		
		scroll->top    = top - 1;
		scroll->bottom = bottom;
		textscroller_gotoxy(s, 0, 0);
	}
}
	
static void 
textscroller_save(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	scroll->s_x = scroll->x;
	scroll->s_y = scroll->y;
	scroll->s_attrfl = scroll->attrfl;
}

static void 
textscroller_restore(scroller_t s)
{
	textscroller_meta *scroll;

	scroll = kgc_scroller_meta(s);

	scroll->x = scroll->s_x;
	scroll->y = scroll->s_y;
	scroll->attrfl = scroll->s_attrfl;	

	textscroller_gotoxy(s, scroll->x, scroll->y);
}

static void 
textscroller_unmap(scroller_t s)
{
	kgi_console_t *cons;

	cons = kgc_scroller_cons(s);

	cons->flags |=  KGI_CF_SPLITLINE;
	cons->flags &= ~KGI_CF_NO_HARDSCROLL;
}

static void 
textscroller_map(scroller_t s)
{
	textscroller_meta *scroll;
	kgi_console_t *cons;
	kgi_u_t render_flags;

	scroll = kgc_scroller_meta(s);
	cons = kgc_scroller_cons(s);
	render_flags = 0;

	RENDER_GET(cons->render, 0, 0, &render_flags);
	if (render_flags & KGI_RF_NO_HARDSCROLL) {
		kgi_s_t org = scroll->origin - scroll->offset;
		if (org < 0) 
			org += scroll->tb.virt.y;

		KRN_ASSERT((0 <= org && org) < scroll->tb.virt.y);
		KRN_DEBUG(3, "using softscroll");

		cons->flags |= KGI_CF_NO_HARDSCROLL | KGI_CF_SPLITLINE;
		org *= scroll->tb.virt.x;
		textscroller_modified(s, org, org + scroll->tb.frame);
	} else {
		cons->flags &= ~KGI_CF_NO_HARDSCROLL;

		if (render_flags & KGI_RF_TILE_Y) {
			cons->flags |= KGI_CF_SPLITLINE;
			KRN_DEBUG(1, "hardscroll & splitline");
		} else {
			cons->flags &= ~KGI_CF_SPLITLINE;
			KRN_DEBUG(1, "hardscroll only");
		}
		KRN_ASSERT(scroll->tb.buf);

		RENDER_PUT_TEXT(cons->render, &scroll->tb, 0, 0, 
				scroll->tb.total);
	}
	textscroller_sync(s);
}

static scroller_method_t textscroller_methods[] = {
	SCROLLMETHOD(scroller_init,		textscroller_init),
	SCROLLMETHOD(scroller_done,		textscroller_done),
	SCROLLMETHOD(scroller_get,		textscroller_get),
	SCROLLMETHOD(scroller_set,		textscroller_set),
	SCROLLMETHOD(scroller_margins,		textscroller_margins),
	SCROLLMETHOD(scroller_map,		textscroller_map),
	SCROLLMETHOD(scroller_unmap,		textscroller_unmap),
	SCROLLMETHOD(scroller_reset,		textscroller_reset),
	SCROLLMETHOD(scroller_erase_display,	textscroller_erase_display),
	SCROLLMETHOD(scroller_gotoxy,		textscroller_gotoxy),
	SCROLLMETHOD(scroller_backward,		textscroller_backward),
	SCROLLMETHOD(scroller_forward,		textscroller_forward),
	SCROLLMETHOD(scroller_update_attr,	textscroller_update_attr),
	SCROLLMETHOD(scroller_cr,		textscroller_cr),
	SCROLLMETHOD(scroller_lf,		textscroller_lf),
	SCROLLMETHOD(scroller_bs,		textscroller_bs),
	SCROLLMETHOD(scroller_hts,		textscroller_hts),
	SCROLLMETHOD(scroller_tbc,		textscroller_tbc),
	SCROLLMETHOD(scroller_ht,		textscroller_ht),
	SCROLLMETHOD(scroller_write,		textscroller_write),
	SCROLLMETHOD(scroller_sync,		textscroller_sync),
	SCROLLMETHOD(scroller_mark,		textscroller_mark),
	SCROLLMETHOD(scroller_modified_mark,	textscroller_modified_mark),
	SCROLLMETHOD(scroller_modified_wrap,	textscroller_modified_wrap),
	SCROLLMETHOD(scroller_mksound,		textscroller_mksound),
	SCROLLMETHOD(scroller_erase_line,	textscroller_erase_line),
	SCROLLMETHOD(scroller_insert_chars,	textscroller_insert_chars),
	SCROLLMETHOD(scroller_delete_chars,	textscroller_delete_chars),
	SCROLLMETHOD(scroller_erase_chars,	textscroller_erase_chars),
	SCROLLMETHOD(scroller_reverse_lf,	textscroller_reverse_lf),
	SCROLLMETHOD(scroller_down,		textscroller_down),
	SCROLLMETHOD(scroller_up,		textscroller_up),
	SCROLLMETHOD(scroller_move,		textscroller_move),
	SCROLLMETHOD(scroller_scroll_top,	textscroller_scroll_top),
	SCROLLMETHOD(scroller_scroll_bottom,	textscroller_scroll_bottom),
	SCROLLMETHOD(scroller_gotox,		textscroller_gotox),
	SCROLLMETHOD(scroller_gotoy,		textscroller_gotoy),
	SCROLLMETHOD(scroller_save,		textscroller_save),
	SCROLLMETHOD(scroller_restore,		textscroller_restore),
	SCROLLMETHOD_END
};

static scroller_driver_t textscroller_driver = {
	"textscroller",
	textscroller_methods,
	sizeof(textscroller_meta)
};

DECLARE_SCROLLER(textscroller, textscroller_driver, textscroller_meta);

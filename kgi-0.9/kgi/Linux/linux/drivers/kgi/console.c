/* ----------------------------------------------------------------------------
**	KGI console driver
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ----------------------------------------------------------------------------
**
**	$Log: console.c,v $
**	Revision 1.3  2001/08/24 19:34:27  seeger_s
**	- fixed missing symbols on compile bug
**	
**	Revision 1.2  2001/07/03 08:50:44  seeger_s
**	- text control now done via image resources
**	- gadget handling now done all in console.c and scroll_sync()
**	
**	Revision 1.1.1.1  2000/04/18 08:50:46  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#define	__CONSOLE_DRIVER__
#define	DEBUG_LEVEL	1

#include <linux/config.h>
#include <linux/version.h>
#include <linux/ascii.h>
#include <linux/major.h>
#include <linux/malloc.h>
#include <linux/vmalloc.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#else
#include <linux/init.h>
#include <linux/devfs_fs_kernel.h>
#endif
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/console.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#define	COMPILING_CONSOLE_C
#include "console.h"
#ifdef	CONFIG_KGI_TERM_XTERM
#include "xterm.h"
#endif

#define	EOK	0
#define	COLOR	1

#define	CONFIG_VT_CONSOLE

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#else
int want_console = -1;

extern void kii_bottomhalf(void); /* FIXME do this properly */
#endif

/*
**	text buffer operations
*/
static inline void tb_move(kgi_u16_t *buf, kgi_u_t dst, kgi_u_t src, kgi_u_t cnt)
{
	memmove(buf + dst, buf + src, cnt + cnt);
}

static inline void tb_set(kgi_u16_t *buf, kgi_u_t src, kgi_u_t val, kgi_u_t cnt)
{
	buf += src;
	cnt--;
	do {
		buf[cnt] = val;

	} while ((kgi_s_t) --cnt >= 0);
}

static inline void tb_invert(kgi_u16_t *buf, kgi_u_t start, kgi_u_t end)
{
	buf += start;
	while (start++ <= end) {

		*buf =     ((*buf) & 0x88FF) 
			| (((*buf)<<4) & 0x7000) | (((*buf>>4)) & 0x0700);
		buf++;
	}
}

/*	color mode attribute to pixel value mapping. We care of the foreground
**	and background colors. The rest is just best effort.
*/
static kgi_u_t tb_atop_color(kgi_u_t attr)
{
	register kgi_u_t val = (attr & KGI_CA_REVERSE)
		? (((attr & 0x0007) << 8) | ((attr & 0x0700) << 4))
		: (((attr & 0x0007) << 12) | (attr & 0x0700));

	if (attr & KGI_CA_BLINK) {

		val |= 0x8000;
	}

	if (attr & KGI_CA_BOLD) {

		val |= 0x0800;
	}

	return val;
}

static kgi_u_t tb_ptoa_color(kgi_u_t val)
{
	return	  ((val & 0x8000) ? KGI_CA_BLINK : KGI_CA_NORMAL)
		| ((val & 0x0800) ? KGI_CA_BOLD  : KGI_CA_NORMAL)
		| ((val & 0x7000) >> 12) | (val & 0x0F00);
}

/*	monochrome attribute mapping. We care about the attributes possible
**	and ignore colors.
*/
static kgi_u_t tb_atop_mono(kgi_u_t attr)
{
	register kgi_u_t
	val = (attr & KGI_CA_UNDERLINE)
		? 0x0100	/* MDA can't do reverse underline :-( */
		: ((attr & KGI_CA_REVERSE) ? 0x7000 : 0x0700);

	if (attr & KGI_CA_BLINK) {

		val |= 0x8000;
	}

	if (attr & KGI_CA_BOLD) {

		val |= 0x0800;
	}

	return val;
}

static kgi_u_t tb_ptoa_mono(kgi_u_t val)
{
	register kgi_u_t
	attr = ((val & 0x7700) == 0x0100)
		? KGI_CA_UNDERLINE
		: (val & 0x7000) ? KGI_CA_REVERSE : KGI_CA_NORMAL;

	if (val & 0x8000) {

		attr |= KGI_CA_BLINK;
	}

	if (val & 0x0800) {

		attr |= KGI_CA_BOLD;
	}

	return attr;
}

static inline void tb_putcs(kgi_console_t *cons, const kgi_u16_t *text,
	kgi_u_t offset, kgi_u_t count)
{
	if (cons->text16) {

		(cons->text16->PutText16)(cons->text16, offset, text, count);
	}
}

/*
**	console font handling
*/
kgi_console_font_t *console_default_font(kgi_console_t *cons)
{
	kgi_u_t i = 0;

	KRN_DEBUG(3, "searching font for height %i", cons->text16->font.y);
	for (i = 0; default_font[i] && 
		(default_font[i]->size.y > cons->text16->font.y); i++) {
	}
	KRN_DEBUG(3, "found font %i (%p)", i, default_font[i]);
	return default_font[i];
}

kgi_u_t console_font_ctop(kgi_console_font_t *font, kgi_isochar_t sym)
{
	kgi_u_t row  = KGI_ISOCHAR_ROW(sym);
	kgi_u_t cell = KGI_ISOCHAR_CELL(sym);
	kgi_console_font_cellinfo_t *cellinfo;
	kgi_u_t l = 0, h, m;

	KRN_ASSERT(font);
	cellinfo = font->info->cellinfo[row];

	if (row == 0xF0) {		/* display directly */

		return cell;
	}

	if ((sym & 0xFFFF0000) || !cellinfo) {

		return font->info->default_pos;
	}

	if (test_bit(row, font->info->map_direct)) {

		return ((kgi_u8_t *) cellinfo) [cell];
	}

	if (sym > cellinfo->sym) {

		return font->info->default_pos;	/* symbol not in list?	*/
	}

	h = cellinfo->pos;
	cellinfo++;

	if (sym < cellinfo->sym) {

		return font->info->default_pos;
	}

	if (cellinfo->sym == sym) {		

		return cellinfo->pos;
	}

	do {	/*	search matching cell
		*/
		if (cellinfo[h].sym == sym) {

			return cellinfo[h].pos;
		}

		m = (--h + l) >> 1;

		if (sym > cellinfo[m].sym) {

			l = m;

		} else { 

			h = m;
		}

	} while (l < h); 

	return font->info->default_pos;	/* no position defined		*/
}

kgi_isochar_t console_font_ptoc(kgi_console_font_t *font, kgi_u_t pos)
{
	return font->info->inversemap[(pos < font->info->positions) ? pos : 0];
}

extern kgi_console_t *console_arr[CONFIG_KGII_MAX_NR_CONSOLES];

/*
**	scroller methods
*/

kii_s_t console_unmap_kii(kii_device_t *dev)
{
	scroll_hide_gadgets((kgi_console_t *) dev->priv.priv_ptr);
	return EOK;
}

void console_map_kii(kii_device_t *dev)
{
	scroll_show_gadgets((kgi_console_t *) dev->priv.priv_ptr);
}

void console_do_kgi_unmap(kgi_console_t *cons)
{
	scroll_hide_gadgets(cons);
	cons->flags |=  KGI_CF_SPLITLINE;
	cons->flags &= ~KGI_CF_NO_HARDSCROLL;
}

void console_do_kgi_map(kgi_console_t *cons)
{
	if (cons->tlut) {

		if (cons->tlut->Set) {

			(cons->tlut->Set)(cons->tlut, 0, 
				0, cons->font->info->positions - 1,
				cons->font->data);
		}
		if (cons->tlut->Select) {

			(cons->tlut->Select)(cons->tlut, 0);
		}
	}

	if (cons->ilut) {

		if (cons->tlut->Set) {

			(cons->ilut->Set)(cons->ilut, 0, 
				0, 16, KGI_AM_COLORS, default_color_text16_ilut);
		}
		if (cons->ilut->Select) {

			(cons->ilut->Select)(cons->ilut, 0);
		}
	}
	if (cons->ptr && 
		(cons->ptr->modes & KGI_MM_3COLOR) &&
		(cons->ptr->size.x == 64 && cons->ptr->size.y == 64)) {

		(cons->ptr->SetMode)(cons->ptr, KGI_MM_3COLOR);
		(cons->ptr->SetShape)(cons->ptr, 0, 
			0,0, default_ptr_64x64, default_ptr_color);
		if (cons->ptr->Select) {

			(cons->ptr->Select)(cons->ptr, 0);
		}
	}

	if (cons->kgi.mode->img[0].virt.y == cons->kgi.mode->img[0].size.y) {

		kgi_s_t org = cons->origin - cons->offset;
		if (org < 0) {

			org += cons->tb_virt.y;
		}
		KRN_ASSERT((0 <= org && org) < cons->tb_virt.y);
		KRN_DEBUG(3, "using softscroll");

		cons->flags |= KGI_CF_NO_HARDSCROLL | KGI_CF_SPLITLINE;
		org *= cons->tb_virt.x;
		scroll_modified(cons, org, org + cons->tb_frame);

	} else {

		cons->flags &= ~KGI_CF_NO_HARDSCROLL;

		if (cons->kgi.mode->img[0].flags & KGI_IF_TILE_Y) {

			cons->flags |= KGI_CF_SPLITLINE;
			KRN_DEBUG(1, "hardscroll & splitline");

		} else {

			cons->flags &= ~KGI_CF_SPLITLINE;
			KRN_DEBUG(1, "hardscroll only");
		}
		KRN_ASSERT(cons->tb_buf);

		tb_putcs(cons, cons->tb_buf, 0, cons->tb_total);
	}
	KRN_ASSERT(cons->kgi.flags & KGI_DF_FOCUSED);
	scroll_sync(cons);
}

void scroll_undo_gadgets(kgi_console_t *cons)
{
	if ((cons->flags & KGI_CF_POINTER_SHOWN) && cons->ptr->Undo) {

		cons->ptr->Undo(cons->ptr);
		cons->flags &= ~KGI_CF_POINTER_SHOWN;
	}
	if ((cons->flags & KGI_CF_CURSOR_SHOWN) && cons->cur->Undo) {

		cons->cur->Undo(cons->cur);
		cons->flags &= ~KGI_CF_CURSOR_SHOWN;
	}
}

void scroll_hide_gadgets(kgi_console_t *cons)
{
	if (cons->flags & KGI_CF_POINTER_SHOWN) {

		KRN_ASSERT(cons->ptr);
		cons->ptr->Hide(cons->ptr);
		cons->flags &= ~KGI_CF_POINTER_SHOWN;
	}
	if (cons->flags & KGI_CF_CURSOR_SHOWN) {

		KRN_ASSERT(cons->cur);
		cons->cur->Hide(cons->cur);
		cons->flags &= ~KGI_CF_CURSOR_SHOWN;
	}
}

void scroll_show_gadgets(kgi_console_t *cons)
{
	if ((cons->flags & KGI_CF_POINTER_SHOWN) && cons->ptr->Undo) {

		cons->ptr->Undo(cons->ptr);
		cons->flags &= ~KGI_CF_POINTER_SHOWN;
	}
	if ((cons->flags & KGI_CF_CURSOR_SHOWN) && cons->cur->Undo) {

		cons->cur->Undo(cons->cur);
		cons->flags &= ~KGI_CF_CURSOR_SHOWN;
	}

	if ((cons->kgi.flags & KGI_DF_FOCUSED) &&
		(cons->kii.flags & KII_DF_FOCUSED) && !cons->offset) {

		if (CONSOLE_MODE(cons, KGI_CM_SHOW_CURSOR) && cons->cur) {

			cons->flags |= KGI_CF_CURSOR_SHOWN;
			cons->cur->Show(cons->cur, cons->x, cons->y);
		}
		if (CONSOLE_MODE(cons, KGI_CM_SHOW_POINTER) && cons->ptr) {

			cons->flags |= KGI_CF_POINTER_SHOWN;
			cons->ptr->Show(cons->ptr,
				cons->kii.ptr.x, cons->kii.ptr.y);
		}

	} else {

		KRN_ASSERT(! (cons->flags & KGI_CF_CURSOR_SHOWN));
		KRN_ASSERT(! (cons->flags & KGI_CF_POINTER_SHOWN));
	}
}

/*	scroll_sync() has to sync the display with the scroller state. The
**	scroller keeps track of the area modified. If there were any 
**	modifications since the last scroll_sync(), the area from cons->from
**	cons->to contained in the currently displayed area needs to be updated.
*/
void scroll_sync(kgi_console_t *cons)
{
	kgi_s_t orig;

	KRN_ASSERT(cons);

	if (! (cons->kgi.flags & KGI_DF_FOCUSED)) {

		return;
	}

	orig = cons->origin - cons->offset;
	if (orig < 0) {

		if ((cons->flags & KGI_CF_NO_HARDSCROLL) ||
			(!(cons->flags & KGI_CF_NO_HARDSCROLL) &&
			  (cons->flags & KGI_CF_SPLITLINE))) {

			orig += cons->tb_virt.y;

		} else {

			orig = 0;
		}
	}
	KRN_ASSERT((kgi_u_t) orig < cons->tb_virt.y);

	/*	when debugging, mark the first line of the textbuffer
	*/
	KRN_TRACE(2, cons->tb_buf[0] = 0x0700 + '=');

	if (cons->flags & KGI_CF_NO_HARDSCROLL) {

		kgi_s_t frm = cons->from;
		kgi_s_t end = cons->to;
		kgi_s_t tb_total = cons->tb_total;

		kgi_s_t back = (orig *= cons->tb_virt.x) + cons->tb_frame;
		kgi_u16_t *tb = cons->tb_buf;
		KRN_ASSERT(tb);

		if (cons->offset || (cons->last_offset != cons->offset) ||
			(cons->flags & KGI_CF_SCROLLED)) {

			frm = orig;
			end = back;
		}

		if ((end < frm) || (end < orig) || (back < frm)) {

			return;
		}

		if (frm < orig) {

			frm = orig;
		}

		if (back < end) {

			end = back;
		}

		scroll_undo_gadgets(cons);

		if (frm < tb_total) {

			if (tb_total >= end) {

				tb_putcs(cons, tb+frm, frm-orig, end-frm);

			} else {

				tb_putcs(cons, tb+frm, frm-orig, tb_total-frm);
				tb_putcs(cons, tb, tb_total-orig, end-tb_total);
			}

		} else {

			tb_putcs(cons, tb+frm-tb_total, frm-orig, end-frm);
		}

		scroll_show_gadgets(cons);

	} else {
/*
		if (cons->flags & KGI_CF_SCROLLED) {

			cons->flags &= ~KGI_CF_SCROLLED;
			kgidev_set_origin(&(cons->kgi), 0, 0, 
				orig*cons->font->size.y);

			if (cons->flags & KGI_CF_SPLITLINE) {

				kgi_s_t split = cons->font->size.y *
					((orig+cons->tb_size.y < cons->tb_virt.y)
						? cons->text16.size.y
						: cons->tb_virt.y - orig);

				kgidev_set_split(&(cons->kgi), split - 1);
			}
		}
*/
	}

	cons->last_offset = cons->offset;
	cons->from = (kgi_u_t) -1L;
	cons->to   = 0;
}

static kgi_s_t scroll_init(kgi_console_t *cons)
{
	kgi_u_t	index = 0;
	kgi_resource_t *resource;
	kgi_marker_t *marker;

	while ((index < __KGI_MAX_NR_IMAGE_RESOURCES) &&
		(resource = cons->kgi.mode->img[0].resource[index])) {

		switch (resource->type) {

		case KGI_RT_TEXT16_CONTROL:
			KRN_DEBUG(2, "text16: %s", resource->name);
			cons->text16 = (kgi_text16_t *) resource;
			break;

		case KGI_RT_CURSOR_CONTROL:
			marker = (kgi_marker_t *) resource;
			if ((NULL == cons->cur) ||
				(cons->cur->Undo && (NULL == marker->Undo))) {

				KRN_DEBUG(2, "cursor: %s", resource->name);
				cons->cur = marker;
			}
			break;

		case KGI_RT_POINTER_CONTROL:
			marker = (kgi_marker_t *) resource;
			if ((NULL == cons->ptr) ||
				(cons->ptr->Undo && (NULL == marker->Undo))) {

				KRN_DEBUG(2, "pointer: %s", resource->name);
				cons->ptr = marker;
			}
			break;

		case KGI_RT_TLUT_CONTROL:
			KRN_DEBUG(2, "tlut: %s", resource->name);
			cons->tlut = (kgi_tlut_t *) resource;
			break;

		case KGI_RT_ILUT_CONTROL:
			KRN_DEBUG(2, "ilut: %s", resource->name);
			cons->ilut = (kgi_ilut_t *) resource;
			break;

		default:
			break;
		}
		index++;
	}
	
	if (! (cons->text16)) {

		KRN_DEBUG(1, "could not get text16 resource "
			"(text16 %p, cur %p, ptr %p)",
			cons->text16, cons->cur, cons->ptr);
		return -EINVAL;
	}

	if (! cons->tb_buf) {

		if (! (cons->tb_buf = vmalloc(CONFIG_KGII_CONSOLEBUFSIZE))) {

			KRN_DEBUG(1, "text buffer allocation failed");
			return -ENOMEM;
		}
		memset(cons->tb_buf, 0, CONFIG_KGII_CONSOLEBUFSIZE);
	}

	cons->tb_size.x = cons->text16->size.x;
	cons->tb_size.y = cons->text16->size.y;

	cons->tb_virt.x = cons->text16->size.x;
	cons->tb_virt.y = CONFIG_KGII_CONSOLEBUFSIZE / 
		(sizeof(*cons->tb_buf)*cons->tb_virt.x);
	if (cons->tb_virt.y < 2*cons->text16->size.y) {

		KRN_DEBUG(1, "text buffer too small, reset failed");
		vfree(cons->tb_buf);
		cons->tb_buf = NULL;
		return -ENOMEM;
	}
	if (cons->tb_virt.y < cons->text16->size.y) {

		cons->kgi.mode->img[0].virt.y = (cons->tb_virt.y *
			cons->kgi.mode->img[0].virt.y) / cons->text16->virt.y;
		cons->text16->virt.y = cons->tb_virt.y;
	}
	if (cons->tb_virt.y > cons->text16->size.y) {

		cons->kgi.mode->img[0].virt.y = (cons->tb_size.y *
			cons->kgi.mode->img[0].virt.y) / cons->text16->virt.y;
		cons->text16->virt.y = cons->tb_size.y;
	}

	cons->tb_frame = cons->tb_virt.x * cons->tb_size.y;
	cons->tb_total = cons->tb_virt.x * cons->tb_virt.y;

	cons->from = cons->tb_total;
	cons->to = 0;
	cons->offset = cons->last_offset = 0;

	cons->flags |= KGI_CF_NO_HARDSCROLL;
	cons->AttrToPixel = COLOR ? tb_atop_color : tb_atop_mono;
	cons->PixelToAttr = COLOR ? tb_ptoa_color : tb_ptoa_mono;
	cons->font = console_default_font(cons);

	CONSOLE_SET_MODE(cons, KGI_CM_SHOW_CURSOR);
#warning finish this!
	KRN_ASSERT(cons->text16->size.x == cons->tb_size.x);
	KRN_ASSERT(cons->text16->size.y == cons->tb_size.y);
	KRN_ASSERT(cons->text16->virt.x == cons->tb_virt.x);

	KRN_DEBUG(4, "%ix%i (%ix%i virt) scroller initialized",
		cons->text16->size.x, cons->text16->size.y,
		cons->text16->virt.x, cons->text16->virt.y);
	return EOK;
}

void scroll_done(kgi_console_t *cons)
{
	if (cons->tb_buf) {

		vfree(cons->tb_buf);
		cons->tb_buf = NULL;
	}
}


#if 0
static kgi_s_t scroll_resize(kgi_console_t *cons, kgi_u_t new_sizex,
	kgi_u_t new_sizey)
{
	kgi_u16_t *src, *dst;
	kgi_u_t new_tb_sizey, i,j;

	KRN_ASSERT(new_sizex);
	KRN_ASSERT(new_sizey);

	new_tb_sizey = CONFIG_KGII_CONSOLEBUFSIZE /
		(sizeof(*cons->tb_buf)*new_sizex);

	if (new_tb_sizey < 2*new_sizey) {

		KRN_DEBUG(1, "text buffer to small, resize failed")
	}

#warning finish!
}
#endif


/*	Scroll up n lines between t(op) and b(ottom) and erase the 'new' bottom
**	area using ERASE. If n is greater than b-t, we scroll only b-t lines.
**	If bottom is more than SIZE_Y, bottom is less or equal top, or n is zero
**	we do nothing.
*/
void scroll_up(kgi_console_t *cons, kgi_u_t t, kgi_u_t b, kgi_u_t n)
{
	kgi_u_t src, end;		/* first and last pixel to move	*/
	kgi_u_t clean, cnt;	/* first pixel/# pixels to clean*/ 
	kgi_u_t dst;		/* pixel to move to		*/
	kgi_u_t h1, h2;		/* temporary variables		*/
	kgi_u_t tb_total;
	kgi_u16_t *buf = cons->tb_buf;

	if ((cons->tb_size.y < b) || (b <= t) || !n) {

		return;
	}

	if (n > (b - t)) {

		n = b - t;
	}

	dst  = t + cons->origin;
	end  = b + cons->origin;

	tb_total = cons->tb_virt.x;

	dst *= tb_total;
	end *= tb_total;

	cnt  = n*tb_total;
	tb_total *= cons->tb_virt.y;
	src  = dst + cnt;

	KRN_ASSERT((dst < src) && (src <= end));

	if (CONSOLE_MODE(cons, KGI_CM_ALT_SCREEN) || t || (b < cons->tb_size.y)) {

		/* can't use origin adjustment	*/
		scroll_modified(cons, dst, end);

		if (end <= src) {

			clean = dst;

		} else {

			clean = end - cnt;

			if (dst >= tb_total) {

				dst -= tb_total;
				src -= tb_total;
				end -= tb_total;
			}

			KRN_ASSERT(dst < tb_total);

			if (end <= tb_total) {

				tb_move(buf, dst, src, end - src);

			} else {

				if (src < tb_total) {

					h1 = tb_total - src;
					tb_move(buf, dst, src, h1);
					src += h1;
					dst += h1;
				}

				h1 = tb_total - dst;
				h2 = end - src;

				KRN_ASSERT((h1 > 0) && (h2 > 0));

				if (h1 < h2) {

					tb_move(buf, dst, src - tb_total, h1);
					tb_move(buf, 0, src - dst, h2 - h1);

				} else {

					tb_move(buf, dst, src - tb_total, h2);
				}
			}
		}

	} else {

		/* adjust origin	*/

		cons->flags |= KGI_CF_SCROLLED;

		if (cons->offset) {

			cons->offset += n;
			if (cons->offset > (cons->tb_virt.y - cons->tb_size.y)) {

				cons->offset = 0;
			}
		}

		cons->origin += n;
		cons->org    += cnt;
		cons->wpos   += cnt;
		cons->pos    += cnt;

		if (tb_total <= cons->pos) {

			cons->pos -= tb_total;
		}

		if (tb_total <= cons->org) {

			cons->org	-= tb_total;
			cons->wpos	-= tb_total;
			cons->origin	-= cons->tb_virt.y;
		}
		clean = cons->org + cons->tb_frame - cnt;
	}

	/* clear bottom area	*/
	scroll_modified(cons, clean, clean + cnt);

	if (clean < tb_total) {

		if (tb_total >= (clean + cnt)) {

			tb_set(buf, clean, cons->erase, cnt);

		} else {

			tb_set(buf, clean, cons->erase, tb_total - clean);
			tb_set(buf, 0, cons->erase, clean + cnt - tb_total);
		}

	} else {

		tb_set(buf, clean - tb_total, cons->erase, cnt);
	}
}


/*	Scroll down n lines between t(op) and b(ottom) and clear the new top
**	area using ERASE. If n is greater than b-t, we scroll only b-t lines.
**	If bottom is more than SIZE_Y, bottom is less or equal top or n is zero
**	we do nothing.
*/
void scroll_down(kgi_console_t *cons, kgi_u_t t, kgi_u_t b, kgi_u_t n)
{
	kgi_u_t src, dst, end;	/* first/last pixel to move	*/
	kgi_u_t cnt;		/* size of area to clean	*/
	kgi_u_t h1, h2;		/* subexpression		*/
	kgi_u_t tb_total;		/* text buffer size		*/
	kgi_u16_t *buf = cons->tb_buf; /* local copy		*/

	if ((cons->tb_size.y < b) || (b <= t) || !n) {

		return;
	}

	if (n > (b - t)) {

		n = b - t;
	}

	end  = t + cons->origin;
	dst  = b + cons->origin;
	tb_total = cons->tb_virt.x;
	end *= tb_total;
	dst *= tb_total;
	cnt  = tb_total*n;
	tb_total *= cons->tb_virt.y;
	src  = dst - cnt;

	KRN_ASSERT((end <= src) && (src < dst));

	scroll_modified(cons, end, dst);

	if (src > end) {

		if (end >= tb_total) {

			dst -= tb_total;
			src -= tb_total;
			end -= tb_total;
		}

		if (dst <= tb_total) {

			tb_move(buf, end + cnt, end, src - end);

		} else {

			if (src > tb_total) {

				h1 = src - tb_total;
				tb_move(buf, dst - src, 0, h1);
				src -= h1;
				dst -= h1;
			}

			h1 = dst - tb_total;
			h2 = src - end;

			if (h1 < h2) {

				tb_move(buf, 0, src - h1, h1);
				h2 -= h1;
				tb_move(buf, tb_total - h2, end, h2);

			} else {

				tb_move(buf, dst - h2 - tb_total, end, h2);
			}
		}
	}

	/* clear top area */

	if (end < tb_total) {

		if (tb_total >= (end + cnt)) {

			tb_set(buf, end, cons->erase, cnt);

		} else {

			tb_set(buf, end, cons->erase, tb_total - end);
			tb_set(buf, 0, cons->erase, end + cnt - tb_total);
		}

	} else {

		tb_set(buf, end - tb_total, cons->erase, cnt);
	}
}




/*	Erase in display using ERASE. The area cleared depends on arg.
*/
void scroll_erase_display(kgi_console_t *cons, kgi_u_t arg)
{
	kgi_u_t src, cnt = cons->tb_frame;	/* area to erase	*/
	kgi_u_t tb_total = cons->tb_virt.x*cons->tb_virt.y;

	switch(arg) {

	case 0:	/* from cursor to end of screen	*/
		src = cons->wpos;
		cnt -= cons->wpos - cons->org;
		break;

	case 1:	/* from start of screen to cursor */
		src = cons->org;
		cnt = cons->wpos + 1 - cons->org;
		break;

	case 2:	/* erase entire screen */
		src = cons->org;
		break;

	default:
		return;
	}

	scroll_modified(cons, src, src + cnt);

	if (src < tb_total) {

		if (tb_total >= (src + cnt)) {

			tb_set(cons->tb_buf, src, cons->erase, cnt);

		} else {

			tb_set(cons->tb_buf, src, cons->erase, tb_total-src);
			tb_set(cons->tb_buf, 0, cons->erase, 
				src + cnt - tb_total);
		}

	} else {

		tb_set(cons->tb_buf, src - tb_total, cons->erase, cnt);
	}

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/*	erase in line using ERASE. The area erased depends on arg.
*/
void scroll_erase_line(kgi_console_t *cons, kgi_u_t arg)
{
	kgi_u_t src, cnt;

	switch (arg) {

	case 0:	/* from cursor to end of line	*/
		src = cons->wpos;
		cnt = cons->tb_virt.x - cons->x;
		break;

	case 1:	/* from line start to cursor	*/
		src = cons->wpos - cons->x;
		cnt = cons->x + 1;
		break;

	case 2:	/* erase entire line	*/
		src = cons->wpos - cons->x;
		cnt = cons->tb_virt.x;
		break;

	default:
		return;
	}

	scroll_modified(cons, src, src + cnt);

	if (cons->tb_total <= src) {

		src -= cons->tb_total;
	}

	tb_set(cons->tb_buf, src, cons->erase, cnt);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}

/*	Insert n ERASE characters.
*/
void scroll_insert_chars(kgi_console_t *cons, kgi_u_t n)
{
	kgi_u_t src = cons->wpos;
	kgi_u_t cnt = cons->tb_virt.x - cons->x;

	KRN_ASSERT((cnt > 0) && (n > 0));

	scroll_modified(cons, src, src + cnt);

	if (cons->tb_total <= src) {

		src -= cons->tb_total;
	}

	if (n < cnt) {

		tb_move(cons->tb_buf, src + n, src, cnt - n);
		cnt = n;
	}

	tb_set(cons->tb_buf, src, cons->erase, cnt);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/*	Delete n characters. Clear the end of the line using ERASE.
*/
void scroll_delete_chars(kgi_console_t *cons, kgi_u_t n)
{
	kgi_u_t cnt = cons->tb_virt.x - cons->x;
	kgi_u_t src = cons->wpos;

	scroll_modified(cons, src, src + cnt);

	if (cons->tb_total <= src) {

		src -= cons->tb_total;
	}

	if (n < cnt) {

		tb_move(cons->tb_buf, src, src + n, cnt - n);
		tb_set(cons->tb_buf, src + cnt - n, cons->erase, n);

	} else {

		tb_set(cons->tb_buf, src, cons->erase, cnt);
	}

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/*	erase the following n chars. (maximum is end of line).
*/
void scroll_erase_chars(kgi_console_t *cons, kgi_u_t n)
{
	KRN_ASSERT(n > 0);
	if (n > (cons->tb_size.x - cons->x)) {

		n = cons->tb_virt.x - cons->x;
	}

	tb_set(cons->tb_buf, cons->pos, cons->erase, n);
	scroll_modified(cons, cons->wpos, cons->wpos + n);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}



/*	Move cursor to the specified position. We have to check all boundaries.
*/
void scroll_gotoxy(kgi_console_t *cons, kgi_s_t new_x, kgi_s_t new_y)
{
	kgi_s_t max_y, min_y;
	kgi_s_t max_x;
#define	min_x	0

	if (CONSOLE_MODE(cons, KGI_CM_ORIGIN)) {

		min_y = cons->top;
		max_y = cons->bottom;

	} else {

		min_y = 0;
		max_y = cons->tb_size.y;
	}

	max_x = cons->tb_size.x;
	cons->x = (new_x < min_x) ? min_x : ((new_x < max_x) ? new_x : max_x-1);
	cons->y = (new_y < min_y) ? min_y : ((new_y < max_y) ? new_y : max_y-1);

	cons->pos = cons->wpos = cons->org + cons->y*cons->tb_virt.x + cons->x;

	if (cons->tb_total <= cons->pos) {

		cons->pos -= cons->tb_total;
	}

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/*	increase the scroll-back offset by <lines> lines. <lines> defaults to
**	half the number of lines of the visible screen if less or equal zero. 
**	As we may be called from a IRQ service, we do not sync the display
**	because this may take some long time. We just mark the CONSOLE_BH and
**	do syncing there.
*/
void scroll_backward(kgi_console_t *cons, int lines)
{
	kgi_u_t max = cons->tb_virt.y - cons->tb_size.y;

	if (lines <= 0) {

		lines = cons->tb_size.y/2;
	}

	cons->offset += lines;
	if (cons->offset > max) {

		cons->offset = max;
	}

	cons->flags |= KGI_CF_SCROLLED;

	scroll_need_sync(cons);
}


/*	decrease the scroll-back offset by <lines> lines. <lines> defaults to
**	half the number of lines of the visible screen if less or equal zero.
**	As we may be called from an IRQ service, we do not sync the display
**	because this may take some long time. We just mark the CONSOLE_BH
**	and do syncing there.
*/
void scroll_forward(kgi_console_t *cons, int lines)
{
	if (lines <= 0) {

		lines = cons->tb_size.y/2;
	}

	cons->offset -= lines;
	if (cons->offset >= cons->tb_total) {

		cons->offset = 0;
	}

	cons->flags |= KGI_CF_SCROLLED;

	scroll_need_sync(cons);
}



/*	Do a linefeed. This means to scroll up one line if the cursor is at
**	the bottom line of the scrolling region or simply to move the cursor
**	down one line.
*/
void scroll_lf(kgi_console_t *cons)
{
	if ((cons->y + 1) == cons->bottom) {

		scroll_up(cons, cons->top, cons->bottom, 1);

	} else {

		if (cons->y < (cons->tb_size.y - 1)) {

			cons->y++;
			cons->wpos += cons->tb_virt.x;
			cons->pos  += cons->tb_virt.x;

			if (cons->tb_total <= cons->pos) {

				cons->pos -= cons->tb_total;
			}
		}
	}

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


/*	Do a reverse linefeed. This means to scroll down one line if the cursor
**	is at the top of the scrolling region or simply to move the cursor up
**	one line.
*/
void scroll_reverse_lf(kgi_console_t *cons)
{
	if (cons->y == cons->top) {

		scroll_down(cons, cons->top, cons->bottom, 1);

	} else {

		if (cons->y > cons->top) {

			cons->y--;
			cons->wpos -= cons->tb_virt.x;

			if (cons->pos < cons->tb_virt.x) {

				cons->pos += cons->tb_total - cons->tb_virt.x;

			} else {

				cons->pos -= cons->tb_virt.x;
			}
		}
	}

	cons->flags &= ~KGI_CF_NEED_WRAP;
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
struct wait_queue *keypress_wait = NULL;
#else
DECLARE_WAIT_QUEUE_HEAD(keypress_wait);
#endif

/*
**	This is a printk() implementation based on the scroll_* functions. 
*/
#ifdef CONFIG_VT_CONSOLE

unsigned int console_printk_console;

static void console_printk(struct console *cp, const char *b, unsigned count)
{
	static int printing = 0;

	kgi_console_t *cons = console_arr[console_printk_console];
	kgi_u_t c;
	kgi_u_t old_pos;

	KRN_ASSERT(cp);

	if (printing || !cons) {

		return;
	}

	printing = 1;

	old_pos = cons->wpos;

	while (count--) {

		c = *(b++);

		if ((c == ASCII_LF) || (c == ASCII_CR) ||
			(cons->flags & KGI_CF_NEED_WRAP)) {

			if (c != ASCII_CR) {

				scroll_modified(cons, old_pos, cons->wpos);
				scroll_lf(cons);
				scroll_sync(cons);
			}

			scroll_cr(cons);
			old_pos = cons->wpos;


			if ((c == ASCII_LF) || (c == ASCII_CR)) {
				continue;
			}
		}

		scroll_write(cons, c);
	}

	if (cons->wpos != old_pos) {

		scroll_modified(cons, old_pos, cons->wpos);
	}
	scroll_sync(cons);
	printing = 0;
}

static int console_wait_key(struct console *cp)
{
	KRN_ASSERT(cp);
	sleep_on(&keypress_wait);
	return 0;
}

static kdev_t console_device(struct console *cp)
{
	KRN_ASSERT(cp);
	return MKDEV(TTY_MAJOR, 1 /* console_printk_console + 1 */);
}

static struct console console_printk_driver =
{
	"tty",				/* name		*/
	console_printk,			/* printk	*/
	NULL,				/* read		*/
	console_device,			/* device	*/
	console_wait_key,		/* wait_key	*/
	NULL,				/* unblank	*/
	NULL,				/* setup	*/
	CON_PRINTBUFFER | CON_ENABLED,	/* flags	*/
	-1,				/* index	*/
	0,				/* cflag	*/
	NULL				/* next		*/
};

#endif /* #ifdef CONFIG_VT_CONSOLE */


#ifdef CONFIG_KGI_TERM_DUMB

/* -----------------------------------------------------------------------------
 *	A very dumb console parser to have output even with no parser loaded.
 * -----------------------------------------------------------------------------
 */

static void dumb_do_reset(kgi_console_t *cons, kgi_u_t do_reset)
{
	struct tty_struct *tty = (struct tty_struct *) cons->kii.tty;

	cons->kii.event_mask = KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT;

	cons->mode = 0;
	CONSOLE_SET_MODE(cons, KGI_CM_SHOW_CURSOR);
	CONSOLE_SET_MODE(cons, KGI_CM_AUTO_WRAP);

	if (tty) {

		tty->winsize.ws_col = cons->tb_size.x;
		tty->winsize.ws_row = cons->tb_size.y;
	}

	cons->origin = cons->org = 0;
	cons->top = 0;
	cons->bottom = cons->tb_size.y;

	cons->bell.pitch = 440 /* Hz */;
	cons->bell.duration = 200 /* msec */;

	cons->attrfl = KGI_CA_NORMAL | 
		KGI_CA_COLOR(KGI_CC_LIGHTGRAY, KGI_CC_BLACK);

	scroll_update_attr(cons);
	scroll_gotoxy(cons, 0, 0);
	scroll_erase_display(cons, 2);

	scroll_sync(cons);

/* !!!	kbd_reset(&(KBD_STATE)); */
	/* !!! kbd_setleds */
}

#endif

static void dumb_handle_kii_event(kii_device_t *dev, kii_event_t *e)
{
	kgi_console_t *cons = (kgi_console_t *) dev->priv.priv_ptr;
	struct tty_struct *tty = (struct tty_struct *) cons->kii.tty;
	
	if (((1 << e->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT)) ||
		(e->key.sym == K_VOID) || !tty) {

		return;
	}

	switch (e->key.sym & K_TYPE_MASK) {

	case K_TYPE_LATIN:
		tty_insert_flip_char(tty, e->key.sym, 0);
		break;

	case K_TYPE_SPECIAL:
		switch (e->key.sym) {

		case K_HOLD:
			tty->stopped ? start_tty(tty) : stop_tty(tty);
			return;

		case K_ENTER:
			tty_insert_flip_char(tty, ASCII_CR, 0);
			break;

		case K_SCROLLFORW:
			scroll_forward(cons, 0);
			break;

		case K_SCROLLBACK:
			scroll_backward(cons, 0);
			break;

		default:
			return;
		}
	}

	wake_up(&keypress_wait);
	tty_schedule_flip(tty);
}

static int dumb_do_write(kgi_console_t *cons, int from_user, const char *buf,
	int count)
{
	struct tty_struct *tty = (struct tty_struct *) cons->kii.tty;
	kgi_u_t cnt = 0;
	kgi_ascii_t c;
	kgi_u_t old_pos = cons->wpos;

	while (! (tty->stopped) && count) {

		if (from_user) {

			if (get_user(c, buf)) {

				return -EFAULT;
			}

		} else {

			c = *buf;
		}
		buf++; cnt++; count--;

		if ((c == ASCII_LF) || (c == ASCII_CR) || 
			(cons->flags & KGI_CF_NEED_WRAP)) {

			if (c != ASCII_CR) {

				scroll_modified(cons, old_pos, cons->wpos);
				scroll_lf(cons);
			}

			scroll_cr(cons);
			old_pos = cons->wpos;

			if ((c == ASCII_LF) || (c == ASCII_CR)) {

				continue;
			}
		}

		if (c == ASCII_BS) {

			scroll_modified(cons, old_pos, cons->wpos);
			scroll_bs(cons);
			old_pos = cons->wpos;
			continue;
		}

		scroll_write(cons, c);
	}

	if (old_pos != cons->wpos) {

		scroll_modified(cons, old_pos, cons->wpos);
	}

	return cnt;
}

/* -----------------------------------------------------------------------------
**	console TTY driver
** -----------------------------------------------------------------------------
*/

#define	DO_RESET	1

/* !!!	This should handle assignment of registered terminal emulators later.
** !!!	For now we only enter the neccessary fields for the dumb or xterm
** !!!	parser.
*/
static int console_assign_parser(kgi_console_t *cons, int do_reset)
{
	KRN_ASSERT(cons);

	cons->kgi.MapDevice	= console_map_kgi;
	cons->kgi.UnmapDevice	= console_unmap_kgi;
	cons->kgi.HandleEvent	= NULL;

	cons->kii.MapDevice	= console_map_kii;
	cons->kii.UnmapDevice	= console_unmap_kii;
#ifdef CONFIG_KGI_TERM_XTERM
	cons->kii.HandleEvent	= &xterm_handle_kii_event;
	cons->DoWrite		= &xterm_do_write;
	xterm_do_reset(cons, do_reset);
#else
	cons->kii.HandleEvent	= &dumb_handle_kii_event;
	cons->DoWrite		= &dumb_do_write;
	dumb_do_reset(cons, do_reset);
#endif
	return EOK;
}

extern int console_need_sync[];

static void console_bottomhalf(void)
{
	int i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
	kii_bottomhalf();
#endif

	for (i = 0; i < CONFIG_KGII_MAX_NR_CONSOLES; i++) {


		if (test_bit(i, console_need_sync)) {

			kgi_console_t *cons = console_arr[i];

			scroll_sync(cons);
			clear_bit(i, console_need_sync);
		}
	}
}

static int console_open(struct tty_struct *tty, struct file *filp)
{
	kgi_u_t index = MINOR(tty->device) - tty->driver.minor_start;
	kgi_console_t *cons = (kgi_console_t *) console_arr[index];
	KRN_ASSERT(filp);

	KRN_DEBUG(3, "open on tty device %i", index);

	if (!cons) {

		KRN_DEBUG(3, "allocating console %i...", index);

		if (! (cons = kmalloc(sizeof(kgi_console_t), GFP_USER))) {

			KRN_DEBUG(1, "failed: not enough memory");
			return -ENOMEM;
		}
		console_arr[index] = cons;
		memset(cons, 0, sizeof(kgi_console_t));

		cons->kgimode.images = 1;
		cons->kgimode.img[0].flags |= KGI_IF_TEXT16;

		cons->kgi.mode = &(cons->kgimode);
		cons->kgi.flags |= KGI_DF_CONSOLE;
		cons->kgi.priv.priv_ptr = cons;

		cons->kii.tty = tty;
		cons->kii.flags |= KII_DF_CONSOLE;
		cons->kii.priv.priv_ptr = cons;

		if (kgi_register_device(&(cons->kgi), index)) {

			KRN_DEBUG(1, "failed: could not register output");
			goto failed0;
		}
		if (kii_register_device(&(cons->kii), index)) {

			KRN_DEBUG(1, "failed: could not register input");
			goto failed1;
		}

		if (scroll_init(cons) || 
			console_assign_parser(cons, DO_RESET)) {

			KRN_DEBUG(1, "failed: could not reset console");
			goto failed2;
		}

		if (! kgi_current_focus(cons->kgi.dpy_id)) {

			kgi_map_device(cons->kgi.id);
			if (! kii_current_focus(cons->kii.focus_id)) {

				kii_map_device(cons->kii.id);
			}
		}
		KRN_DEBUG(4, "console %i allocated.", index);
	}

	KRN_ASSERT(tty->count > 0);
	if (tty->count == 1) {

		KRN_DEBUG(3, "linking console %i to tty", index);
		cons->refcnt++;
		cons->kii.tty = tty;
		tty->driver_data = cons;

	} else {

		KRN_ASSERT(tty->driver_data == cons);
		KRN_ASSERT(cons->kii.tty == tty);
	}

	if (!tty->winsize.ws_row && !tty->winsize.ws_col) {

		tty->winsize.ws_row = cons->tb_size.y;
		tty->winsize.ws_col = cons->tb_size.x;
	}
	return EOK;

failed2:scroll_done(cons);
	kii_unregister_device(&(cons->kii));
failed1:kgi_unregister_device(&(cons->kgi));
failed0:kfree(cons);
	console_arr[index] = NULL;
	return -ENXIO;
}

static void console_close(struct tty_struct *tty, struct file *filp)
{
	kgi_u_t index = MINOR(tty->device) - tty->driver.minor_start;
	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;
	KRN_ASSERT(filp);
	KRN_ASSERT(cons == console_arr[index]);

	KRN_DEBUG(3, "close on tty device %i", index);

	if (cons) {

		KRN_ASSERT(tty->count > 0);
		if (tty->count == 1) {

			cons->refcnt--;
			cons->kii.tty = NULL;
			tty->driver_data = NULL;
		}
		if (cons->refcnt == 0) {

			KRN_DEBUG(3, "freeing console %i", cons->kgi.id);

			scroll_undo_gadgets(cons);
			if (cons->kii.flags & KII_DF_FOCUSED) {

				kii_unmap_device(cons->kii.id);
			}
			if (cons->kgi.flags & KGI_DF_FOCUSED) {

				kgi_unmap_device(cons->kgi.id);
			}
			kii_unregister_device(&(cons->kii));
			kgi_unregister_device(&(cons->kgi));
			console_arr[index] = tty->driver_data = NULL;

			scroll_done(cons);
			kfree(cons);
			console_arr[index] = tty->driver_data = NULL;
		}
	}
}

static void console_start(struct tty_struct *tty)
{
	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;

	cons->kii.flags |= KII_DF_SCROLL_LOCK;
	kiidev_sync(&(cons->kii), KII_SYNC_LED_FLAGS);
}

static void console_stop(struct tty_struct *tty)
{
	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;

	cons->kii.flags &= ~KII_DF_SCROLL_LOCK;
	kiidev_sync(&(cons->kii), KII_SYNC_LED_FLAGS);
}

static int console_write(struct tty_struct *tty, int from_user, 
	const unsigned char *buf, int count)
{
	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;
	int cnt;

	cnt = cons->DoWrite(cons, from_user, buf, count);
	scroll_sync(cons);

	return cnt;
}

static void console_put_char(struct tty_struct *tty, unsigned char c)
{
	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;

	cons->DoWrite(cons, 0, &c, 1);
	scroll_sync(cons);
}

static void console_flush_char(struct tty_struct *tty)
{
	kgi_console_t *cons = (kgi_console_t *) tty->driver_data;

	scroll_show_gadgets(cons);
}

static int console_write_room(struct tty_struct *tty)
{
	return tty->stopped ? 0 : PAGE_SIZE;
}

static int console_chars_in_buffer(struct tty_struct *tty)
{
	KRN_ASSERT(tty);
	return 0;
}

static int			console_refcount;
static struct tty_struct *console_table[CONFIG_KGII_MAX_NR_CONSOLES];
static struct termios	 *console_termios[CONFIG_KGII_MAX_NR_CONSOLES];
static struct termios	 *console_termios_locked[CONFIG_KGII_MAX_NR_CONSOLES];

static struct tty_driver console_driver;

static inline void console_driver_init(void)
{
	memset(&console_driver, 0, sizeof(struct tty_driver));

	console_driver.magic		= TTY_DRIVER_MAGIC;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
	console_driver.name             = "tty";
#else
	console_driver.driver_name	= "vc_emulator";
	console_driver.name		= "vc/%d";
       /* Tell tty_register_driver() to skip consoles because they are
        * registered before kmalloc() is ready. We'll patch them in later.
        * See comments at console_init(); see also con_init_devfs().
        */
	console_driver.flags            |= TTY_DRIVER_NO_DEVFS;
#endif
	console_driver.name_base	= 1;
	console_driver.major		= TTY_MAJOR;
	console_driver.minor_start	= 1;
	console_driver.num		= CONFIG_KGII_MAX_NR_CONSOLES;
	console_driver.type		= TTY_DRIVER_TYPE_CONSOLE;
	console_driver.init_termios	= tty_std_termios;
	console_driver.flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_RESET_TERMIOS;
	console_driver.refcount		= &console_refcount;
	console_driver.table		= console_table;
	console_driver.termios		= console_termios;
	console_driver.termios_locked	= console_termios_locked;

	console_driver.open		= console_open;
	console_driver.close		= console_close;
	console_driver.write		= console_write;
	console_driver.write_room	= console_write_room;
	console_driver.put_char		= console_put_char;
	console_driver.flush_chars	= console_flush_char;
	console_driver.chars_in_buffer	= console_chars_in_buffer;
#ifdef CONFIG_KGI_VT_LINUX
	vt_init();
	console_driver.ioctl		= vt_ioctl; 
#else
	console_driver.ioctl		= NULL;
#endif
	console_driver.stop		= console_stop;
	console_driver.start		= console_start;
}

int	console_need_sync[(CONFIG_KGII_MAX_NR_CONSOLES+sizeof(int)*8-1)/(sizeof(int)*8)];
kgi_console_t *console_arr[CONFIG_KGII_MAX_NR_CONSOLES];

#define	PANIC(x)	panic(__FILE__ ": " x);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
void dev_console_init(void)
#else 
void __init dev_console_init(void)
#endif
{
	kgi_console_t *cons;

	console_driver_init();
	memset(console_need_sync, 0, sizeof(console_need_sync));
	memset(console_arr, 0, sizeof(console_arr));
	init_bh(CONSOLE_BH, console_bottomhalf);

	if (tty_register_driver(&console_driver)) {

		PANIC("Could not register console driver!\n");
	}

	if (kgi_display_registered(0) == KGI_EOK) {

		if (!(cons = kmalloc(sizeof(kgi_console_t),
			GFP_KERNEL))) {

			PANIC("Could not allocate console struct!\n");
		}
		console_arr[0] = cons;
		memset(cons, 0, sizeof(*cons));

		cons->kgimode.images = 1;

		cons->kgi.mode = &(cons->kgimode);
		cons->kgi.flags |= KGI_DF_CONSOLE;
		cons->kgi.priv.priv_ptr = cons;

		cons->kii.tty = current->tty;
		cons->kii.flags |= KII_DF_CONSOLE;
		cons->kii.priv.priv_ptr = cons;

		if (kgi_register_device(&(cons->kgi), 0)) {

			PANIC("Could not register output!\n");
		}
		if (kii_register_device(&(cons->kii), 0)) {

			PANIC("Could not register input!\n");
		}
		if (scroll_init(cons)) {

			PANIC("Could not reset scroller state!\n");
		}
		if (console_assign_parser(cons, DO_RESET)) {

			PANIC("Could not assign console parser!\n");
		}

		cons->refcnt++;

/* #		ifdef CONFIG_VT_CONSOLE */

			console_printk_console = 0;
			register_console(&console_printk_driver);
			KRN_DEBUG(2, "console printk driver registered.");
/* #		endif */

		kgi_map_device(cons->kgi.id);
	}
	kii_map_device(console_arr[0]->kii.id);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#else

#ifdef CONFIG_KGI_VT_LINUX
/* We can't register the console with devfs during dev_console_init(),
 * because it is called before kmalloc() works.  This function is called
 * later to do the registration.
 */
void __init con_init_devfs (void)
{
	int i;

	for (i = 0; i < console_driver.num; i++)
		tty_register_devfs (&console_driver, DEVFS_FL_AOPEN_NOTIFY,
				    console_driver.minor_start + i);
}
#endif

#endif



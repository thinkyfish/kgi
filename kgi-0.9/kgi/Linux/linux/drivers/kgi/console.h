/* ----------------------------------------------------------------------------
**	KGI console implementation specific definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: console.h,v $
**	Revision 1.1.1.1  2000/04/18 08:50:47  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _console_h
#define	_console_h

#include <linux/types.h>
#include <linux/vt.h>
#include <linux/malloc.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/kgii.h>
#include <kgi/kgi.h>
#include <kgi/cmd.h>
#define	KII_NEED_MODIFIER_KEYSYMS
#include <kii/kii.h>

typedef struct
{
	unsigned char	kbmeta;
	unsigned char	mode;
	unsigned char	kbmode;
#define	VC_RAW		0
#define	VC_XLATE	1
#define	VC_MEDIUMRAW	2
#define	VC_UNICODE	3

	unsigned char	kbled;
	unsigned char	led;

	pid_t		spawnpid;
	unsigned long	spawnsig;

} kgi_console_kd_t;

typedef enum
{
	KGI_CM_SMOOTH_SCROLL,	/* smooth(slow) scroll		*/
	KGI_CM_REVERSE_VIDEO,	/* black on white		*/
	KGI_CM_ORIGIN,		/* cursor origin mode		*/
	KGI_CM_AUTO_WRAP,	/* automatic line wrap		*/
	KGI_CM_AUTO_REPEAT,	/* repeat chars automatically	*/
	KGI_CM_REVERSE_WRAP,	/* allow reverse wrapping	*/
	KGI_CM_ALT_SCREEN,	/* use alternate screen		*/
	KGI_CM_SHOW_CURSOR,	/* show/hide cursor mark	*/
	KGI_CM_SHOW_POINTER,	/* show/hide pointer mark	*/

        KGI_CM_LAST /* NOTE: there *must* not be more than 32 modes! */

} kgi_console_mode_t;

#define	CONSOLE_CLEAR_MODE(cons, m)	((cons)->mode &= ~(1 << (m)))
#define	CONSOLE_SET_MODE(cons, m)	((cons)->mode |=  (1 << (m)))
#define	CONSOLE_MODE(cons, m)		((cons)->mode &   (1 << (m)))

typedef enum
{
#define	KGI_CA_COLOR(fg,bg)	(((bg) & 0xFF) | (((fg) & 0xFF) << 8))
	KGI_CA_FG_COLOR	= 0x0000FF00,
	KGI_CA_BG_COLOR = 0x000000FF,
	KGI_CA_NORMAL	= 0x00000000,
	KGI_CA_HALF	= 0x00010000,
	KGI_CA_BRIGHT	= 0x00020000,
	KGI_CA_INTENSITY= 0x00030000,
	KGI_CA_UNDERLINE= 0x00040000,
	KGI_CA_BOLD	= 0x00080000,
	KGI_CA_ITALIC	= 0x00100000,
	KGI_CA_REVERSE	= 0x00200000,
	KGI_CA_BLINK	= 0x00400000

} kgi_console_attributes_t;

typedef enum
{
	KGI_CF_NEED_WRAP	= 0x00000001,
	KGI_CF_SCROLLED		= 0x00000002,

	KGI_CF_NO_HARDSCROLL	= 0x00000004,
	KGI_CF_SPLITLINE	= 0x00000008,

	KGI_CF_CURSOR_SHOWN	= 0x00000010,
	KGI_CF_CURSOR_TO_SHOW	= 0x00000020,
	KGI_CF_POINTER_SHOWN	= 0x00000040,
	KGI_CF_POINTER_TO_SHOW	= 0x00000080

} kgi_console_flags_t;


/*	console fonts
**
**	The console fonts have (at maximum) 256 font positions as this can
**	(hopefully) be done or emulated on all displays. To support ISO10646 as
**	good and fast as possible, we use a (speed and common case) optimized
**	mapping not to exhaust memory:
**
**	Generally cellinfo[ISO_ROW(x)] points to data about row ISO_ROW(x):
**
**	  * NULL if the font has no positions that map to this row.
**	  * if bit number row of map_direct is set, cellinfo[row] is a pointer
**	    to an 'kgi_u8 map[256]' array, map[ISO_CELL(sym)] giving the font
**	    position. Use this if more than 31 font postions map to this row.
**	  * else cellinfo[row] points to an array kgi_font_cellinfo's, sorted
**	    in ascending order of cellinfo.sym and the first giving total
**	    number of cells and highest cell assigned.
*/

typedef struct
{
	kgi_isochar_t	sym;
	kgi_u_t		pos;

} kgi_console_font_cellinfo_t;

typedef struct
{
	kgi_u_t		positions;	/* number of positions defined	*/
	kgi_u_t		default_pos;	/* position to use as default	*/
	kgi_u_t		map_direct[8];
	void		*cellinfo[256];	/* map info isochar->fontpos	*/
	kgi_isochar_t	inversemap[256];/* map info fontpos->isochar	*/

} kgi_console_font_info_t;

typedef struct
{
	kgi_ucoord_t	size;		/* distance between char's	*/
	kgi_ucoord_t	bitmap;		/* bitmap size			*/
	kgi_u_t		base;		/* base line			*/

	kgi_console_font_info_t *info;	/* font info			*/
	kgi_u8_t	*data;		/* font data			*/

} kgi_console_font_t;

typedef struct kgi_console_s kgi_console_t;

struct kgi_console_s
{
	kgi_u_t	refcnt;
	kii_device_t	kii;
	kgi_device_t	kgi;
	kgi_mode_t	kgimode;

	/*	scroller state
	*/
	int	(*DoWrite)(kgi_console_t *, int, const char *, int);
	kgi_u_t	(*AttrToPixel)(kgi_u_t attr);
	kgi_u_t	(*PixelToAttr)(kgi_u_t pval);

	/*	text buffer controls
	*/
	kgi_text16_t	*text16;
	kgi_ilut_t	*ilut;
	kgi_tlut_t	*tlut;
	kgi_marker_t	*cur;
	kgi_marker_t	*ptr;

	kgi_u16_t	*tb_buf;	/* background/scrollback buffer	*/
	kgi_u_t		tb_total;	/* total characters in buffer	*/
	kgi_u_t		tb_frame;	/* characters per visible frame	*/
	kgi_ucoord_t	tb_size;	/* visible size of buffer	*/
	kgi_ucoord_t	tb_virt;	/* virtual size of buffer	*/

	/*	miscellaneous scroller variables
	*/
	kgi_u_t		from, to;
	kgi_u_t		offset;
	kgi_u_t		last_offset;
	kgi_u_t		top, bottom;
	kgi_u_t		origin;
	kgi_u_t		org;
	kgi_u_t		x,y;
	kgi_u_t		wpos;
	kgi_u_t		pos;

	kgi_u_t		attrfl;
	kgi_u_t		fg,bg;
	kgi_u_t		attr;
	kgi_u_t		erase;
	struct {
		kgi_u_t pitch, duration;

	}		bell;

	kgi_console_flags_t	flags;
	kgi_console_mode_t	mode;
	kgi_console_font_t	*font;

	/*	compatibility mode stuff
	*/
	unsigned int	vc_mode;
	kgi_console_kd_t	kd;
	struct vt_mode	vt_mode;
	pid_t		vt_pid;
	unsigned int	vt_ack;
#define	VT_ACK_IDLE		0
#define	VT_ACK_PENDING_FROM	1
#define	VT_ACK_PENDING_TO	2
#define	VT_ACK_DONE		3

};


enum kgi_console_color_e
{
	KGI_CC_BLACK=0,	KGI_CC_BLUE,	KGI_CC_GREEN,	KGI_CC_CYAN,
	KGI_CC_RED,	KGI_CC_MAGENTA,	KGI_CC_BROWN,	KGI_CC_LIGHTGRAY,
	KGI_CC_DARKGRAY,KGI_CC_LIGHTBLUE,KGI_CC_LIGHTGREEN,KGI_CC_LIGHTCYAN,
	KGI_CC_LIGHTRED,KGI_CC_LIGHTMAGENTA,KGI_CC_YELLOW,KGI_CC_WHITE
};


/*	the scroller interface
*/
#ifdef __KERNEL__

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
extern struct wait_queue *keypress_wait;
#else
extern wait_queue_head_t keypress_wait;
#endif

#ifdef CONFIG_KGI_VT_LINUX
extern int vt_handle_kii_event(kgi_console_t *cons, kii_event_t *ev);
extern void vt_reset_vc(kgi_console_t *cons);
extern int vt_ioctl(struct tty_struct *tty, struct file *file, unsigned int cmd, unsigned long arg);
extern void vt_init(void);
#endif

extern void scroll_up(kgi_console_t *cons, kgi_u_t t, kgi_u_t b, kgi_u_t n);
extern void scroll_down(kgi_console_t *cons, kgi_u_t t, kgi_u_t b, kgi_u_t n);
extern void scroll_erase_display(kgi_console_t *cons, kgi_u_t arg);
extern void scroll_erase_line(kgi_console_t *cons, kgi_u_t arg);
extern void scroll_insert_chars(kgi_console_t *cons, kgi_u_t n);
extern void scroll_delete_chars(kgi_console_t *cons, kgi_u_t n);
extern void scroll_erase_chars(kgi_console_t *cons, kgi_u_t n);
extern void scroll_backward(kgi_console_t *cons, kgi_s_t lines);
extern void scroll_forward(kgi_console_t *cons, kgi_s_t lines);
extern void scroll_lf(kgi_console_t *cons);
extern void scroll_reverse_lf(kgi_console_t *cons);
extern void scroll_sync(kgi_console_t *cons);
extern void scroll_gotoxy(kgi_console_t *cons, kgi_s_t new_x, kgi_s_t new_y);
extern void scroll_hide_gadgets(kgi_console_t *cons);
extern void scroll_undo_gadgets(kgi_console_t *cons);
extern void scroll_show_gadgets(kgi_console_t *cons);

extern kgi_console_font_t *	default_font[CONFIG_KGII_MAX_NR_DEFFONTS];
extern kgi_rgb_color_t		default_color[16];
extern kgi_rgb_color_t		default_ptr_color[3];
extern kgi_u8_t			default_ptr_64x64[1024];

extern kgi_isochar_t console_font_ptoc(kgi_console_font_t *f, kgi_u_t p);
extern kgi_u_t console_font_ctop(kgi_console_font_t *f, kgi_isochar_t c);

extern void console_do_kgi_map(kgi_console_t *cons);
extern void console_do_kgi_unmap(kgi_console_t *cons);
extern void console_map_kgi(kgi_device_t *dev);
extern kgi_s_t console_unmap_kgi(kgi_device_t *dev);

extern int console_need_sync[(CONFIG_KGII_MAX_NR_CONSOLES+sizeof(int)*8-1)/(sizeof(int)*8)];
extern void dev_console_init(void);

/*	There is a bug in GCC that disables checking for warnings in
**	'extern inline' functions. So for we force inline functions to be
**	declared 'extern' and not to be inlined to trap silly errors not
**	found otherwise when DEBUG is declared!
*/
#ifdef DEBUG_LEVEL

#define	INLINE 

extern void scroll_modified(kgi_console_t *cons, kgi_u_t from, kgi_u_t to);
extern void scroll_update_attr(kgi_console_t *cons);
extern void scroll_cr(kgi_console_t *cons);
extern void scroll_bs(kgi_console_t *cons);
extern void scroll_write(kgi_console_t *cons, kgi_isochar_t c);
extern void scroll_need_sync(kgi_console_t *cons);
extern void scroll_mksound(kgi_console_t *cons, kgi_u_t pitch, kgi_u_t duration);
#else

#define	INLINE	extern inline

#endif

#if (! defined(DEBUG_LEVEL) || defined(COMPILING_CONSOLE_C))

INLINE void scroll_modified(kgi_console_t *cons, kgi_u_t from, kgi_u_t to)
{
	KRN_ASSERT(from <= to);

	if (from < cons->from) {

		cons->from = from;
	}

	if (cons->to < to) {

		cons->to = to;
	}
}

INLINE void scroll_update_attr(kgi_console_t *cons)
{
	struct tty_struct *tty = cons->kii.tty;

	cons->erase = cons->attr = cons->AttrToPixel(cons->attrfl);
	cons->erase |= console_font_ctop(cons->font,
		tty ? ERASE_CHAR(tty) : 0x20);
}

INLINE void scroll_cr(kgi_console_t *cons)
{
	cons->wpos -= cons->x;
	cons->pos -= cons->x;
	cons->x = 0;
	cons->flags &= ~KGI_CF_NEED_WRAP;
}

INLINE void scroll_bs(kgi_console_t *cons)
{
	if (cons->x) {

		cons->x--;
		cons->wpos--;
		cons->pos--;
		cons->flags &= ~KGI_CF_NEED_WRAP;
	}
}

INLINE void scroll_write(kgi_console_t *cons, kgi_isochar_t c)
{
	cons->tb_buf[cons->pos] = cons->attr | console_font_ctop(cons->font, c);

	if (cons->x == (cons->tb_size.x - 1)) {

		cons->flags |= KGI_CF_NEED_WRAP;
		return;
	}

	cons->x++;
	cons->wpos++;
	cons->pos++;
}

INLINE void scroll_need_sync(kgi_console_t *cons)
{
	set_bit(cons->kgi.id, console_need_sync);
	mark_bh(CONSOLE_BH);
}

INLINE void scroll_mksound(kgi_console_t *cons, kgi_u_t pitch, kgi_u_t duration)
{
	KRN_DEBUG(0, "scroll_mksound(%p, %i, %i) not implemented yet.",
		cons, pitch, duration);
}

#endif	/* #if (!defined(DEBUG) || defined(COMPILING_CONSOLE_C)) */

#endif	/* #ifdef __KERNEL__ */

#endif	/* #ifndef _console_h */

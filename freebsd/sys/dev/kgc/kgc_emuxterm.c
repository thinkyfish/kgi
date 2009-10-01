/*-
 * Copyright (c) 1996-2000 Steffen Seeger
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
 * xterm console parser
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	1
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/consio.h>
#include <sys/fbio.h>
#include <sys/kernel.h>
#include <sys/tty.h>
#include <dev/fb/fbreg.h>

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kii/kii.h>
#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_scroller.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_gfbrndr.h>
#include <dev/kgc/kgc_emuxterm.h>
#include "scroller_if.h"
#include "render_if.h"

#undef	STRICT_XTERM	/* #define to restrict to original XTERM	*/
#undef	SETEC_ASTRONOMY	/* #define to force display of concealed text	*/

#define TRACE(x)	KRN_TRACE(2,(x))

static kgi_u8_t DEFAULT_COLORS[8] = {
	KGI_CC_LIGHTGRAY,	KGI_CC_BLACK, /* Normal.	*/
	KGI_CC_LIGHTRED,	KGI_CC_BLACK, /* Blink.		*/
	KGI_CC_LIGHTBLUE,	KGI_CC_BLACK, /* Underline.	*/
};

enum color_index {
	CI_FG,		 CI_BG,
	CI_BLINK_FG, CI_BLINK_BG,
	CI_UL_FG,	 CI_UL_BG
};

static kgi_u8_t colormap[16] = {
	KGI_CC_BLACK,		KGI_CC_RED,
	KGI_CC_GREEN,		KGI_CC_BROWN,
	KGI_CC_BLUE,		KGI_CC_MAGENTA,
	KGI_CC_CYAN,		KGI_CC_LIGHTGRAY,
	KGI_CC_DARKGRAY,	KGI_CC_LIGHTRED,
	KGI_CC_LIGHTGREEN,	KGI_CC_YELLOW,
	KGI_CC_LIGHTBLUE,	KGI_CC_LIGHTMAGENTA,
	KGI_CC_LIGHTCYAN,	KGI_CC_WHITE
};

#define	DEFAULT_FG	(DEFAULT_COLORS[CI_FG])
#define	DEFAULT_BG	(DEFAULT_COLORS[CI_BG])

#define CTRL_ACTION 0x0d00ff81	/* chars<32 that invoke special action	*/
#define	CTRL_ALWAYS 0x0800f581	/* can not be overridden by DISP_CTRL	*/

/*
 * Forward declaration of the font translation tables. See the end of this
 * file for the initializer.
 */
static kgi_unicode_t translation[4][256];

static void 
xterm_set_charset(kgi_console_t *cons, kgi_u_t nr, char c)
{
	kgi_console_xterm_t *xterm;

 	xterm = (kgi_console_xterm_t *)cons;

	switch(c) {
	case 'K':		/* ??? original console code did this?	*/
	case 'A':		/* United Kingdom/national		*/
		xterm->g[nr] = USER_MAP;
		break;
	case '1':		/* 'alternate ROM standard character'	*/
	case 'B':		/* US-ASCII (ISO-8859-1 = Latin1)	*/
		xterm->g[nr] = LAT1_MAP;
		break;
	case '2':		/* 'alternate ROM graphics'		*/
	case '0':		/* VT100 graphics			*/
		xterm->g[nr] = GRAF_MAP;
		break;
	case 'U':		/* ??? original console code did this?	*/
		xterm->g[nr] = IBMPC_MAP;
		break;
	}

	if (xterm->charset == nr) 
		xterm->translate = translation[xterm->g[nr]];
}

enum {	
	cursor, id, status, 
	mouseup, 
	mousedown1, mousedown2, mousedown3
};

static void 
xterm_do_set_mode(kgi_console_t *cons, kgi_s_t mode, kgi_s_t on)
{
	kgi_u_t attrfl, erase;

	if (mode == KGI_CM_AUTO_REPEAT) {
		if (on)
			cons->kii.event_mask |= KII_EM_KEY_REPEAT;
		else 
			cons->kii.event_mask &= ~KII_EM_KEY_REPEAT;
	}

	if (mode == KGI_CM_REVERSE_VIDEO) {
		if (!CONSOLE_MODE(cons, KGI_CM_REVERSE_VIDEO) != !on) {
/* XXX #warning correct inversion, invert alternate screen too! */
#if 0
			fb_invert(FB, 0, cons->fb_size - 1);
			SCROLLER_MODIFIED(cons->scroller, cons->org,
				cons->org + cons->fsize);
#endif
			SCROLLER_GET(cons->scroller, 0, 0, 0, 0, 0, &attrfl, &erase); 
			attrfl ^= KGI_CA_REVERSE;
			SCROLLER_SET(cons->scroller, attrfl, erase); 
		}
	}


#if 0	/* !!! implement alternate screen	*/
	if ((mode == alt_screen) && (!MODE(alt_screen) != !on)) {

		int cnt = FB_SIZE_X*SIZE_Y;

		if (CONSOLE_MODE(cons, KGI_CM_ALT_SCREEN)) {

			if (ORG + cnt < FB_SIZE) {

				fb_memcpy_tofb(FB, ALT_SCREEN, ORG, cnt);

           		} else {

				fb_memcpy_tofb(FB, ALT_SCREEN, ORG,
					FB_SIZE-ORG);
				fb_memcpy_tofb(FB, ALT_SCREEN+FB_SIZE-ORG,
					0, ORG+cnt-FB_SIZE);
			}

		} else {

			if (ORG + cnt < FB_SIZE) {

				fb_memcpy_fromfb(FB, ALT_SCREEN, ORG, cnt);

           		} else {

				fb_memcpy_fromfb(FB, ALT_SCREEN, ORG,
					FB_SIZE - ORG);
				fb_memcpy_fromfb(FB, ALT_SCREEN + FB_SIZE-ORG,
					0, ORG+cnt-FB_SIZE);
			}
		}
	}
#endif

	/*
	 * XXX if (mode == column132 && MODE(allow_132mode))
	 * XXX con_resize(cons, SIZE_Y, on ? 132 : 80);
	 */

	if ((mode == KGI_CM_XT_X10_MOUSE) || (mode == KGI_CM_XT_REPORT_MOUSE) ||
			(mode == KGI_CM_XT_TRACK_MOUSE)) {
		if (on)
			CONSOLE_SET_MODE(cons, KGI_CM_SHOW_POINTER);
		else
			CONSOLE_CLEAR_MODE(cons, KGI_CM_SHOW_POINTER);
	}

	if (on)
		CONSOLE_SET_MODE(cons, mode);
	else 
		CONSOLE_CLEAR_MODE(cons, mode);

	SCROLLER_UPDATE_ATTR(cons->scroller);

	if (mode == KGI_CM_ORIGIN) 
		SCROLLER_GOTOXY(cons->scroller, 0, 0);
}

static int 
xterm_par_to_mode(register int ques, register int par)
{

	if (ques) {
		switch (par) {
		case 1:
			return (KGI_CM_XT_CURSOR_KEY);
		case 3:
			return (KGI_CM_XT_COLUMN132);
		case 4:
			return (KGI_CM_SMOOTH_SCROLL);
		case 5:
			return (KGI_CM_REVERSE_VIDEO);
		case 6:
			return (KGI_CM_ORIGIN);
		case 7:
			return (KGI_CM_AUTO_WRAP);
		case 8:
			return (KGI_CM_AUTO_REPEAT);
		case 9:
			return (KGI_CM_XT_X10_MOUSE);
		case 38:
			return (KGI_CM_XT_TEXTRONIX_MODE);
		case 40:
			return (KGI_CM_XT_ALLOW_132MODE);
		case 41:
			return (KGI_CM_XT_CURSES_FIX);
		case 44:
			return (KGI_CM_XT_MARGIN_BELL);
		case 45:
			return (KGI_CM_REVERSE_WRAP);
		case 46:
			return (KGI_CM_XT_LOGGING);
		case 47:
			return (KGI_CM_ALT_SCREEN);
		case 1000:
			return (KGI_CM_XT_REPORT_MOUSE);
		case 1001:
			return (KGI_CM_XT_TRACK_MOUSE);
		}
	} else {
		switch (par) {
		case 3:
			return (KGI_CM_XT_DISPLAY_CTRL);
		case 4:
			return (KGI_CM_XT_INSERT);
		case 20:
			return (KGI_CM_XT_NEWLINE);
		}
	}

	return (-1);
}

static void 
xterm_set_mode(kgi_console_t *cons, int on)
{
	kgi_console_xterm_t *xterm;
	kgi_u_t i;
	kgi_s_t mode;

	xterm = (kgi_console_xterm_t *)cons;

	for (i = 0; i <= xterm->npar; i++) {
		if ((mode = xterm_par_to_mode(cons->flags & KGI_CF_XT_QUESTION,
			xterm->par[i])) >= 0) {
			xterm_do_set_mode(cons, mode, on);
		}
	}

	RENDER_HIDE_GADGETS(cons->render);
	cons->flags &= ~(KGI_CF_CURSOR_TO_SHOW | KGI_CF_POINTER_TO_SHOW);

	if (CONSOLE_MODE(cons, KGI_CM_SHOW_CURSOR)) 
		cons->flags |= KGI_CF_CURSOR_TO_SHOW;

	if (CONSOLE_MODE(cons, KGI_CM_XT_REPORT_MOUSE) |
		CONSOLE_MODE(cons, KGI_CM_XT_X10_MOUSE) |
		CONSOLE_MODE(cons, KGI_CM_XT_TRACK_MOUSE)) {
		cons->flags |= KGI_CF_POINTER_TO_SHOW;
	}
}

static void 
xterm_backup_mode(kgi_console_t *cons, int save)
{
	kgi_console_xterm_t *xterm;
	kgi_u_t i;
	kgi_s_t mode;

	xterm = (kgi_console_xterm_t *)cons;

	for (i = 0; i <= xterm->npar; i++) {
		if ((mode = xterm_par_to_mode(cons->flags & KGI_CF_XT_QUESTION, 
			xterm->par[i])) >= 0) {
			kgi_u_t mask = 1 << mode;

			if (save) {
				xterm->s_mode &= ~mask;
				xterm->s_mode |= cons->mode & mask;
			} else {
				if ((cons->mode & mask) != (xterm->s_mode & mask))
					xterm_do_set_mode(cons, mode, xterm->s_mode & mask);
			}
		}
	}

	RENDER_HIDE_GADGETS(cons->render);
	cons->flags &= ~(KGI_CF_CURSOR_TO_SHOW | KGI_CF_POINTER_TO_SHOW);

	if (CONSOLE_MODE(cons, KGI_CM_SHOW_CURSOR)) 
		cons->flags |= KGI_CF_CURSOR_TO_SHOW;

	if (CONSOLE_MODE(cons, KGI_CM_XT_REPORT_MOUSE) |
		CONSOLE_MODE(cons, KGI_CM_XT_X10_MOUSE) |
		CONSOLE_MODE(cons, KGI_CM_XT_TRACK_MOUSE)) {
		cons->flags |= KGI_CF_POINTER_TO_SHOW;
	}
}

static void 
xterm_save_cur(kgi_console_t *cons)
{
	kgi_console_xterm_t *xterm; 
	kgi_u_t i;

	xterm = (kgi_console_xterm_t *)cons;

	/*
	 * XXX
	 * Warning save reverse_mode too!
     */
	SCROLLER_SAVE(cons->scroller);

	xterm->s_charset = xterm->charset;

	for (i = 0; i < sizeof(xterm->s_g); i++) 
		xterm->s_g[i] = xterm->g[i];

	for (i = 0; i < sizeof(xterm->s_colors); i++) 
		xterm->s_colors[i] = xterm->colors[i];
}


static void 
xterm_restore_cur(kgi_console_t *cons)
{
	kgi_console_xterm_t *xterm;
	kgi_u_t i;

	xterm = (kgi_console_xterm_t *)cons;
	xterm->charset = xterm->s_charset;

	for (i = 0; i < sizeof(xterm->g); i++)
		xterm->g[i] = xterm->s_g[i];

	for (i = 0; i < sizeof(xterm->colors); i++)
		xterm->colors[i] = xterm->s_colors[i];

	xterm->translate = translation[xterm->g[xterm->charset]];

	SCROLLER_RESTORE(cons->scroller);
	SCROLLER_UPDATE_ATTR(cons->scroller);
	cons->flags &= ~KGI_CF_NEED_WRAP;
}


static void 
xterm_set_term(kgi_console_t *cons)
{
	kgi_console_xterm_t *xterm;

	xterm = (kgi_console_xterm_t *)cons;

	switch(xterm->par[0]) {
	case 10:
		if (xterm->npar >= 1) {
			xterm->bell.pitch = ((xterm->par[1] < 20) ||
				(xterm->par[1] > 32767)) ? 0 : xterm->par[1];
		} else {	
			xterm->bell.pitch = 750;
		}
		break;
	case 11:
		if (xterm->npar >= 1)
			xterm->bell.duration = (xterm->par[1] < 2000) ? xterm->par[1] : 0;
		else
			xterm->bell.duration = 125;
		break;
	}
}

static void 
csi_m(kgi_console_t *cons)
{
	kgi_console_xterm_t *xterm;
	kgi_u_t i, attrfl, erase;

	xterm = (kgi_console_xterm_t *)cons;

	SCROLLER_GET(cons->scroller, 0, 0, 0, 0, 0, &attrfl, &erase); 

	for (i = 0; i <= xterm->npar; i++) {
		switch (xterm->par[i]) {
		case 0:
			attrfl = KGI_CA_NORMAL | KGI_CA_COLOR(DEFAULT_FG, DEFAULT_BG);
			break;
		case 1:
			attrfl &= ~KGI_CA_INTENSITY;
			attrfl |= KGI_CA_BOLD;
			break;
#ifndef STRICT_XTERM
		case 2:
			attrfl &= ~KGI_CA_INTENSITY;
			attrfl |= KGI_CA_HALF;
			break;
#endif
		case 4:
			attrfl |= KGI_CA_UNDERLINE;
			break;
		case 5:
			attrfl |= KGI_CA_BLINK;
			break;
		case 7:
			if (CONSOLE_MODE(cons, KGI_CM_REVERSE_VIDEO))
				attrfl &= ~KGI_CA_REVERSE;
			else
				attrfl |= KGI_CA_REVERSE;
			break;
#ifndef STRICT_XTERM
#	ifndef SETEC_ASTRONOMY
		case 8:
			attrfl &= ~KGI_CA_FG_COLOR;
			attrfl |= (attrfl << 8) & KGI_CA_FG_COLOR;
			break;
#	endif
		case 10:
			/*
			 * ANSI X3.64-1979 (SCO-ish?)
			 * Select primary font, don't display
			 * control chars if defined, don't set
			 * bit 8 on output.
			 */
			xterm->translate = translation[xterm->g[xterm->charset]];
			CONSOLE_CLEAR_MODE(cons, KGI_CM_XT_DISPLAY_CTRL);
			CONSOLE_CLEAR_MODE(cons, KGI_CM_XT_TOGGLE_META);
			break;
		case 11:
			/*
			 * ANSI X3.64-1979 (SCO-ish?)
			 * Select first alternate font, let's
			 * chars < 32 be displayed as ROM chars.
			 */
			xterm->translate = translation[IBMPC_MAP];
			CONSOLE_SET_MODE(cons, KGI_CM_XT_DISPLAY_CTRL);
			CONSOLE_CLEAR_MODE(cons, KGI_CM_XT_TOGGLE_META);
			break;
		case 12:
			/*
			 * ANSI X3.64-1979 (SCO-ish?)
			 * Select second alternate font, toggle
			 * high bit before displaying as ROM char.
			 */
			xterm->translate = translation[IBMPC_MAP];
			CONSOLE_SET_MODE(cons, KGI_CM_XT_DISPLAY_CTRL);
			CONSOLE_SET_MODE(cons, KGI_CM_XT_TOGGLE_META);
			break;
#endif
		case 21: /* Fall thru. */
		case 22:
			attrfl &= ~KGI_CA_INTENSITY;
			attrfl |= KGI_CA_NORMAL;
			break;
		case 24:
			attrfl &= ~KGI_CA_UNDERLINE;
			break;
		case 25:
			attrfl &= ~KGI_CA_BLINK;
			break;
		case 27:
			if (CONSOLE_MODE(cons, KGI_CM_REVERSE_VIDEO))
				attrfl |= KGI_CA_REVERSE;
			else 
				attrfl &= ~KGI_CA_REVERSE;
			break;
#ifndef STRICT_XTERM
		case 38:
			/*
			 * ANSI X3.64-1979 (SCO-ish?)
			 * Enables underscore, white foreground
			 * with white underscore (Linux - use
			 * default foreground).
			 */
			attrfl |= KGI_CA_UNDERLINE;
			attrfl &= ~0xFF00;
			attrfl |= KGI_CC_WHITE << 8;
			break;
		case 39:
			/*
			 * ANSI X3.64-1979 (SCO-ish?)
			 * ???	Disable underline option. Reset color
			 * ???	to default?
			 * It did this before:
			 */
			attrfl &= ~KGI_CA_UNDERLINE;
			attrfl &= ~KGI_CA_FG_COLOR;
			attrfl |= DEFAULT_FG << 8;
			break;	
		case 49:
			attrfl &= ~KGI_CA_BG_COLOR;
			attrfl |= DEFAULT_BG;
			break;
#endif
		default:
			if ((xterm->par[i] >= 30) && (xterm->par[i] <= 37)) {
				attrfl &= ~KGI_CA_FG_COLOR;
				attrfl |= ((unsigned long) colormap[xterm->par[i]-30]) << 8; 
			} else {
				if ((xterm->par[i] >= 40) && (xterm->par[i] <= 47)) {	
					attrfl &= ~KGI_CA_BG_COLOR;
					attrfl |= ((unsigned long) colormap[xterm->par[i]-40]);
				}
			}
			break;
		}
	}

	SCROLLER_SET(cons->scroller, attrfl, erase); 
	SCROLLER_UPDATE_ATTR(cons->scroller);
}


void 
xterm_do_reset(kgi_console_t *cons, int do_reset)
{
	kgi_u_t attrfl, erase;

#if 0
	struct tty *tp = (struct tty *) cons->kii.tty;
	kgi_ucoord_t render_size, scroller_size;
#endif
	kgi_console_xterm_t *xterm = (kgi_console_xterm_t *)cons;

#ifdef notyet
	cons->kii.event_mask = KII_EM_POINTER | KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT;
#endif
	cons->kii.event_mask = KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT;

#if 0
	SCROLLER_GET(cons->scroller, &scroller_size, 0, 0, 0, 0, 0, 0);
	RENDER_GET(cons->render, &render_size, 0, 0);

	if (tp) {

		tp->t_winsize.ws_col = scroller_size.x;
		tp->t_winsize.ws_row = scroller_size.y;

		tp->t_winsize.ws_xpixel = render_size.x;
		tp->t_winsize.ws_ypixel = render_size.y;
	}

	kiidev_set_pointer_window(&cons->kii,
		0, scroller_size.x * cons->text16->cell.x,
		0, scroller_size.y * cons->text16->cell.y);
#endif

	SCROLLER_GET(cons->scroller, 0, 0, 0, 0, 0, &attrfl, &erase); 

	cons->mode	= 0;
	CONSOLE_SET_MODE(cons, KGI_CM_SHOW_CURSOR);
	CONSOLE_SET_MODE(cons, KGI_CM_AUTO_WRAP);
	CONSOLE_SET_MODE(cons, KGI_CM_XT_META_ESC);
	xterm->s_mode = cons->mode;

	cons->flags &= ~(KGI_CF_XT_QUESTION | KGI_CF_XT_CAPS_LOCK | 
		KGI_CF_XT_CAPS_STICKY | KGI_CF_XT_NUM_LOCK);

	xterm->state	= ESnormal;
	xterm->utf_char	= 0;
	xterm->utf_count= 0;

	xterm->bell.pitch = 750 /* Hz */;
	xterm->bell.duration = 125 /* msec */;

	xterm->charset	= 0;
	xterm->g[0]	= LAT1_MAP;
	xterm->g[1]	= GRAF_MAP;
	xterm->g[2]	= IBMPC_MAP; 
	xterm->g[3]	= USER_MAP;

	xterm->translate = translation[xterm->g[xterm->charset]];

	xterm->npar = 0;

	attrfl = KGI_CA_NORMAL | 
		KGI_CA_COLOR(KGI_CC_LIGHTGRAY, KGI_CC_BLACK);

	SCROLLER_SET(cons->scroller, attrfl, erase); 

	memcpy(xterm->colors, DEFAULT_COLORS, sizeof(xterm->colors));

	xterm_save_cur(cons);

	if (do_reset) {
		SCROLLER_RESET(cons->scroller);
		SCROLLER_UPDATE_ATTR(cons->scroller);
		SCROLLER_GOTOXY(cons->scroller, 0, 0);
		SCROLLER_ERASE_DISPLAY(cons->scroller, 2);
		SCROLLER_SYNC(cons->scroller);
	}
}

static inline void 
xterm_put_char(kgi_console_t *cons, kgi_ascii_t c)
{
	struct tty *tp;
	
	tp = cons->kii.tty;
	tty_lock(tp);
	ttydisc_rint(tp, (char)c, 0);
	ttydisc_rint_done(tp);
	tty_unlock(tp);
}

static inline void 
xterm_application_key(kgi_console_t *cons, kgi_ascii_t key,	kgi_u_t mode)
{

	xterm_put_char(cons, ASCII_ESC);
	xterm_put_char(cons, mode ? 'O' : '[');
	xterm_put_char(cons, key);
}

static void 
xterm_function_key(kgi_console_t *cons, kgi_u_t key)
{
	const kgi_ascii_t *s; 

	s = kiidev_get_fnstring(&cons->kii, key);
	if (s) {
		while (*s)
			xterm_put_char(cons, *(s++));
	}
}

static const char *cur = "BDCA";
static const char *pad = "0123456789+-*/\015,.?";
static const char *app = "pqrstuvwxylSRQMnn?";

static void 
xterm_numpad_key(kgi_console_t *cons, kgi_u8_t val, kii_u_t shift)
{

	if (CONSOLE_MODE(cons, KGI_CM_XT_APPLIC_KEY) && !(shift & KII_MM_SHIFT)) {
		xterm_application_key(cons, app[val], 1);
		return;
	}

	if (!(cons->flags & KGI_CF_XT_NUM_LOCK)) {
		switch(K_SYM(K_TYPE_NUMPAD, val)) {
		case K_PCOMMA: /* Fall thru. */
		case K_PDOT:
			xterm_function_key(cons, K_VALUE(K_REMOVE));
			return;
		case K_P0:
			xterm_function_key(cons, K_VALUE(K_INSERT));
			return;
		case K_P1:
			xterm_function_key(cons, K_VALUE(K_SELECT));
			return;
		case K_P2:
			xterm_application_key(cons, cur[K_VALUE(K_DOWN)], 0);
			return;
		case K_P3:
			xterm_function_key(cons, K_VALUE(K_PGDN));
			return;
		case K_P4:
			xterm_application_key(cons, cur[K_VALUE(K_LEFT)], 0);
			return;
		case K_P6:
			xterm_application_key(cons, cur[K_VALUE(K_RIGHT)], 0);
			return;
		case K_P7:
			xterm_function_key(cons, K_VALUE(K_FIND));
			return;
		case K_P8:
			xterm_application_key(cons, cur[K_VALUE(K_UP)], 0);
			return;
		case K_P9:
			xterm_function_key(cons, K_VALUE(K_PGUP));
			return;
		}
	}

	xterm_put_char(cons, pad[val]);
	if ((val == K_VALUE(K_PENTER)) && CONSOLE_MODE(cons, KGI_CM_XT_NEWLINE))
		xterm_put_char(cons, ASCII_LF);	
}

#if 0
static void xterm_report(kgi_console_t *cons, kgi_u_t what)
{
	char buf[40], *s = buf;
	char cbutton = ' ';
#	define VT100ID	"\033[?1;2c"	/* VT100 with advanced video option */
#	define VT102ID	"\033[?6c"

#	define MOUSEX	('!' + (char)(cons->kii.ptr.x/cons->text16->cell.x))
#	define MOUSEY	('!' + (char)(cons->kii.ptr.y/cons->text16->cell.y))

	switch (what) {

		case cursor:
			sprintf(buf, "\033[%hd;%hdR",
				cons->y + (CONSOLE_MODE(cons, KGI_CM_ORIGIN)
					? cons->top+1 : 1), cons->x+1);
			break;

		case id:
			s = VT102ID;
			break;
 			
		case status:
			s = "\033[0n";
			break;

		case mouseup:
			sprintf(buf, "\033[M#%c%c", MOUSEX, MOUSEY);
			break;

		case mousedown3:
			cbutton++;
		case mousedown2:
			cbutton++;
		case mousedown1:
			sprintf(buf, "\033[M%c%c%c", cbutton, MOUSEX, MOUSEY);
			break;
	}

	while (*s) {

		xterm_put_char(cons, *(s++));
	}
	tty_schedule_flip((struct tty_struct *) cons->kii.tty);

#	undef	MOUSEX
#	undef	MOUSEY
}
#endif

void 
xterm_handle_kii_event(kii_device_t *dev, kii_event_t *ev)
{
	kgi_console_t *cons;
	register kgi_u_t sym;		
	struct tty *tp;

	cons = dev->priv.priv_ptr;
#if 0
	if ((1 << ev->any.type) & KII_EM_POINTER) {

		switch (ev->any.type) {

		case KII_EV_PTR_BUTTON_PRESS:

			if (CONSOLE_MODE(cons, KGI_CM_XT_REPORT_MOUSE) ||
				CONSOLE_MODE(cons, KGI_CM_XT_X10_MOUSE)) {

				if (ev->pbutton.button & 1) {

					xterm_report(cons, mousedown1);
				}
				if (ev->pbutton.button & 4) {

					xterm_report(cons, mousedown2);
				}
				if (ev->pbutton.button & 2) {

					xterm_report(cons, mousedown3);
				}
			}
			break;

		case KII_EV_PTR_BUTTON_RELEASE:

			if (CONSOLE_MODE(cons, KGI_CM_XT_REPORT_MOUSE)) {

				xterm_report(cons, mouseup);
			}
			break;

		case KII_EV_PTR_RELATIVE:
		case KII_EV_PTR_ABSOLUTE:
			RENDER_SHOW_GADGETS(cons->render);
			break;
		}
	}
#endif

	if ((1 << ev->any.type) & (KII_EM_KEYBOARD)) {	
		tp = cons->kii.tty;
		sym = ev->key.sym;
	
		if (tp == NULL)
			return;

		switch(K_TYPE(sym)) {
		case K_TYPE_META:
			if (CONSOLE_MODE(cons, KGI_CM_XT_META_ESC)) {
				xterm_put_char(cons, ASCII_ESC);
				xterm_put_char(cons, K_VALUE(sym));
			} else {
				xterm_put_char(cons, K_VALUE(sym) | 0x80);
			}
			break;
		case K_TYPE_FUNCTION:
			xterm_function_key(cons, K_VALUE(sym));
			break;
		case K_TYPE_SPECIAL:
			switch (sym) {
			case K_ENTER:
				xterm_put_char(cons, ASCII_CR);
				if (CONSOLE_MODE(cons, KGI_CM_XT_NEWLINE)) 
					xterm_put_char(cons, ASCII_LF); 
				break;
#if 0
			case K_BREAK:
				tty_insert_flip_char(tty, 0, TTY_BREAK);
				break;
#endif					
			case K_CAPS:
				/*
				 * XXX where to handle LEDs? XXX
				 * See kii/kii.c
				 */
				return;
			case K_NUM:
				if (CONSOLE_MODE(cons, KGI_CM_XT_APPLIC_KEY)) {
					xterm_application_key(cons, 'P', 1);
				} else {
					cons->flags ^= KGI_CF_XT_NUM_LOCK;
					/*
					 * XXX where to handle LEDs? XXX
					 * See kii/kii.c
					 */
					return;
				}
				break;
#ifdef notyet
			case K_HOLD:
				if (tp->stopped) {
					start_tty(tp);
				} else {
					stop_tty(tp);
				}
				return;
#endif
			case K_SCROLLFORW:
				SCROLLER_FORWARD(cons->scroller, 0);
				SCROLLER_SYNC(cons->scroller);
				return;
			case K_SCROLLBACK:
				SCROLLER_BACKWARD(cons->scroller, 0);
				SCROLLER_SYNC(cons->scroller);
				return;
			case K_CAPSON:
				/*
				 * XXX handled in kii.c
				 * XXX where to handle LEDs?
				 */
				return;
			case K_BARENUMLOCK:
				cons->flags ^= KGI_CF_XT_NUM_LOCK;
				/* XXX where to handle LEDs? XXX */
				return;
			default:
				return;
			}
			break;
		case K_TYPE_NUMPAD:
			xterm_numpad_key(cons, K_VALUE(sym), ev->key.normal);
			break;
		case K_TYPE_CURSOR:
			xterm_application_key(cons, cur[K_VALUE(sym)],
					CONSOLE_MODE(cons, KGI_CM_XT_CURSOR_KEY));
			break;
		case K_TYPE_SHIFT: /* Fall thru. */
		case K_TYPE_CONSOLE: /* Fall thru. */
		case K_TYPE_ASCII: /* Fall thru. */
		case K_TYPE_DEAD: /* Fall thru. */
			return;
		default:
			if (CONSOLE_MODE(cons, KGI_CM_XT_UTF8)) {
				if (sym < 0x80) {
					xterm_put_char(cons, sym);
				} else if (sym < 0x800) {
					xterm_put_char(cons, 0xC0 | (sym >> 6));
					xterm_put_char(cons, 0x80 | (sym & 0x3F));
				} else {
					xterm_put_char(cons, 0xE0 | (sym >> 12));
					xterm_put_char(cons, 0x80 | (sym >> 6));
					xterm_put_char(cons, 0x80 | (sym & 0x3F));
				}
				/*
				 * UTF-8 is defined to up to 31 bits,
				 * but as the keyboard delivers unicode
				 * chars only we only need 16 bit here.
				 */
			} else {
				if (K_TYPE(sym) == K_TYPE_LATIN)
					xterm_put_char(cons, K_VALUE(sym));
			}
		}
	}
}

int 
xterm_do_write(kgi_console_t *cons, const char *buf, int count)
{
	kgi_console_xterm_t *xterm;
	kgi_u_t n, erase, attrfl, top, bottom;
	kgi_isochar_t c;  /* 8-bit character to process. */
	kgi_isochar_t tc; /* Translated character.		 */
	kgi_u_t printable;

	n = 0;
 	xterm = (kgi_console_xterm_t *)cons;

	SCROLLER_MARK(cons->scroller);

	while (count) {
		c = *buf;
		c &= 0xff; /* !!! Make positive. */
		buf++; n++; count--;

		/*
		 * XXX 
		 * Shouldn't toggle_meta mode *change* bit 0x80
		 */
		tc = xterm->translate[CONSOLE_MODE(cons, KGI_CM_XT_TOGGLE_META)
			? (c | 0x80) : c];

		printable = (c >= 32) || ((1 << c) & 
			(CONSOLE_MODE(cons, KGI_CM_XT_DISPLAY_CTRL)
				? ~CTRL_ALWAYS : ~CTRL_ACTION));

		if (printable && (xterm->state == ESnormal)) {
			if (cons->flags & KGI_CF_NEED_WRAP) {
				SCROLLER_MODIFIED_MARK(cons->scroller);
				if (CONSOLE_MODE(cons, KGI_CM_AUTO_WRAP)) 
					SCROLLER_LF(cons->scroller);

				SCROLLER_CR(cons->scroller);
				SCROLLER_MARK(cons->scroller);
			}

			if (CONSOLE_MODE(cons, KGI_CM_XT_INSERT)) {
				TRACE(printf("<INS>"));
				SCROLLER_INSERT_CHARS(cons->scroller, 1);
			}

			TRACE(printf("%.2x %.8x     ", c, cons->mode));
			SCROLLER_WRITE(cons->scroller, tc);
			continue;
		}

		/*
		 * Control characters can be used in the *middle* of
		 * an escape sequence.
		 */
		switch (c) {
		case ASCII_BEL:
			TRACE(printf("<BEL>"));
			if (xterm->bell.pitch && xterm->bell.duration) {
				SCROLLER_MKSOUND(cons->scroller, xterm->bell.pitch,
						xterm->bell.duration);
			}
			continue;	
		case ASCII_BS:
			TRACE(printf("<BS>"));
			SCROLLER_MODIFIED_MARK(cons->scroller);
			SCROLLER_BS(cons->scroller);
			SCROLLER_MARK(cons->scroller);
			continue;
		case ASCII_HT:
			TRACE(printf("<HT>"));
			SCROLLER_MODIFIED_MARK(cons->scroller);
			SCROLLER_HT(cons->scroller);
			SCROLLER_MARK(cons->scroller);
			continue;
		case ASCII_LF: /* Fall thru. */
		case ASCII_VT: /* Fall thru. */
		case ASCII_FF: /* Fall thru. */
			TRACE(printf("<LF>"));
			SCROLLER_MODIFIED_MARK(cons->scroller);
			SCROLLER_LF(cons->scroller);
			SCROLLER_MARK(cons->scroller);	
			if (!CONSOLE_MODE(cons, KGI_CM_XT_NEWLINE)) 
				continue;
		case ASCII_CR:
			TRACE(printf("<CR>"));
			SCROLLER_MODIFIED_MARK(cons->scroller);
			SCROLLER_CR(cons->scroller);
			SCROLLER_MARK(cons->scroller);
			continue;
		case ASCII_SO:
			TRACE(printf("<S0>"));
			xterm->charset = 1;
			xterm->translate = translation[xterm->g[1]];
			CONSOLE_SET_MODE(cons, KGI_CM_XT_DISPLAY_CTRL);
			continue;
	 	case ASCII_SI:
			TRACE(printf("<SI>"));
			xterm->charset = 0;
			xterm->translate = translation[xterm->g[0]];
			CONSOLE_CLEAR_MODE(cons, KGI_CM_XT_DISPLAY_CTRL);
			continue;
		case ASCII_CAN: /* Fall thru. */
		case ASCII_SUB:
			TRACE(printf("<CAN>"));
			xterm->state = ESnormal; /* XXX display checkerboard! */
			continue;
		case ASCII_ESC:
			TRACE(printf("<ESC"));
			xterm->state = ESesc;
			continue;
		case 127:
			TRACE(printf("<DEL>"));
			/* XXX SCORLLER_DEL(cons->scroller); */
			continue;
		case 128+27:
			TRACE(printf("<CSI"));	
			xterm->state = ESsquare;
			continue;
		}

		switch(xterm->state) {
		case ESesc:
			TRACE(printf("%c", (char) c));
			xterm->state = ESnormal;
			/*
			 * Handle sequences that do not change cursor
			 * position first.
			 */
			switch(c) {
			case '[':
				xterm->state = ESsquare;
				continue;
			case '>':	/* DECKPNM - numeric keypad */
				TRACE(printf("= DECKPNM>\n"));
				CONSOLE_CLEAR_MODE(cons, KGI_CM_XT_APPLIC_KEY);
				continue;
			case '=':	/* DECKPAM - appl. keypad */
				TRACE(printf("= DECKPAM>\n"));
				CONSOLE_SET_MODE(cons, KGI_CM_XT_APPLIC_KEY);
			 	continue;
			case '(':	/* SCS G0 - select character set 0 */
				TRACE(printf("= SCS0>\n"));
				xterm->state = ESset;
				xterm->par[0] = 0;	/* charset to set */
				continue;
			case ')':	/* SCS G1 - select character set 1 */
				TRACE(printf("= SCS1>\n"));
				xterm->state = ESset;
				xterm->par[0] = 1;
				continue;
			case '*':	/* SCS G2 - select character set 2 */
				TRACE(printf("= SCS2>\n"));
				xterm->state = ESset;
				xterm->par[0] = 2;
				continue;
			case '+':	/* SCS G3 - select character set 3 */
				TRACE(printf("= SCS3>\n"));
				xterm->state = ESset;
				xterm->par[0] = 3;
				continue;
			case '#':
				xterm->state = EShash;
				continue;
			case 'H':	/* HTS - set horiz. tab stop */
				TRACE(printf("= HTS>\n"));
				SCROLLER_HTS(cons->scroller);
				continue;
			case 'N':
				TRACE(printf("= S2>\n"));
				xterm->charset = 2;
				xterm->translate = translation[xterm->g[2]];
				CONSOLE_CLEAR_MODE(cons,KGI_CM_XT_DISPLAY_CTRL);
				continue;
			case 'O':
				TRACE(printf("= S3>\n"));
				xterm->charset = 3;
				xterm->translate = translation[xterm->g[3]];
				CONSOLE_CLEAR_MODE(cons,KGI_CM_XT_DISPLAY_CTRL);
				continue;
#if 0
			case 'Z':	/* DECID - report ID */
				TRACE(printf("= DECID>\n"));
				xterm_report(cons, id);
				continue;
#endif
			case 'n':  /* XXX full locking shift select G2 set */
			case 'o':  /* XXX full locking shift select G3 set */
				continue;
			case '7':	/* DECSC - save cursor */
				TRACE(printf("= DECSC>\n"));
				xterm_save_cur(cons);
				continue;
#ifndef STRICT_XTERM
			case '%':
				xterm->state = ESpercent;
				continue;
#endif
			}

			/*
			 * Now care of sequences that (potentially) alter
			 * the cursor position. Note that *all* cases have
			 * to be terminated with 'break'.
			 */
			SCROLLER_MODIFIED_MARK(cons->scroller);

			switch (c) {
			case 'E':	/* NEL - next line */
				TRACE(printf("= NEL>\n"));
				SCROLLER_CR(cons->scroller);
				SCROLLER_LF(cons->scroller);
				break;
			case 'M':	/* RI - reverse index */
				TRACE(printf("= RI>\n"));
				SCROLLER_REVERSE_LF(cons->scroller);
				break;
			case 'D':	/* IND - index (down one line) */
				TRACE(printf("= IND>\n"));
				SCROLLER_LF(cons->scroller);
				break;
			case '8':	/* DECRC - restore cursor */
				TRACE(printf("= DECRC>\n"));
				xterm_restore_cur(cons);
				break;
			case 'c':	/* RIS - reset to initial state */
				TRACE(printf("= RIS>\n"));
				xterm_do_reset(cons, 1);
				break;
			default:
				TRACE(printf("= unknown ESC>\n"));
			}
			SCROLLER_MARK(cons->scroller);
			continue;			
		case ESsquare:
			for (xterm->npar = 0; xterm->npar < XTERM_MAX_NPAR; xterm->npar++) 
				xterm->par[xterm->npar] = 0;

			xterm->npar = 0;
			xterm->state = ESgetpars;

			if (c == '[') {		/* Function key */
				TRACE(printf("%c", (char) c));
				xterm->state = ESfunckey;
				continue;
			}
			if (c == '?') {
				TRACE(printf("%c", (char) c));
				cons->flags |= KGI_CF_XT_QUESTION;
				continue;
			}
			/* Fall through if ((c != '[') && (c != '?')) */
		case ESgetpars:
			if ((c == ';') && (xterm->npar < XTERM_MAX_NPAR-1)) {
				TRACE(printf("%c", (char) c));
				xterm->npar++;
				continue;
			} else {
				if ((c >= '0') && (c <= '9')) {
					TRACE(printf("%c", (char) c));
					xterm->par[xterm->npar] *= 10;
					xterm->par[xterm->npar] += c-'0';
					continue;
				} else {
					xterm->state = ESgotpars;
				}
			}
			/* Fall through when parameters are complete! */
		case ESgotpars:
			xterm->state = ESnormal;
			TRACE(printf("%c", (char) c));

			if (cons->flags & KGI_CF_XT_QUESTION) {
				/*
				 * All these sequences may alter cursor
				 * position. Terminate cases with 'break'!
				 */
				SCROLLER_MODIFIED_MARK(cons->scroller);
				switch(c) {
				case 'h':	/* SM - set mode */
					TRACE(printf("= SM>\n"));
					xterm_set_mode(cons, 1);
					break;
				case 'l':	/* RM - reset mode */
					TRACE(printf("= RM>\n"));
					xterm_set_mode(cons, 0);
					break;
				case 'r':	/* DECRM - restore mode */
					TRACE(printf("= DECRM>\n"));	
					xterm_backup_mode(cons, 0);
					break;
				case 's':	/* DECSM - save mode */
					TRACE(printf("= DECSM>\n"));
					xterm_backup_mode(cons, 1);
					break;
				default:
					TRACE(printf("= unknown ESC[? >\n"));
				}
				SCROLLER_MARK(cons->scroller);

				cons->flags &= ~KGI_CF_XT_QUESTION;
				continue;
			}

			/*
			 * First care of sequences that do not alter
			 * cursor postion.
			 */
			switch(c) {
			case 'J':	/* ED - erase display	*/
				TRACE(printf("= ED>\n"));
				SCROLLER_ERASE_DISPLAY(cons->scroller, xterm->par[0]);
				continue;
			case 'K':	/* EL - erase in line */
				TRACE(printf("= EL>\n"));
				SCROLLER_ERASE_LINE(cons->scroller, xterm->par[0]);
				continue;

			case 'P':
				TRACE(printf("= ?_?>\n"));
				if (xterm->par[0] <= 0) 
					xterm->par[0] = 1;
				SCROLLER_DELETE_CHARS(cons->scroller, xterm->par[0]);
				continue;
#if 0
			case 'c':	/* ??? DA - device attributes */
				TRACE(printf("= DA>\n"));
				if (!xterm->par[0]) {

					xterm_report(cons, id);
				}
				continue;
#endif
			case 'g':	/* TBC - tabulation clear */
				TRACE(printf("= TBC>\n"));
				SCROLLER_TBC(cons->scroller, xterm->par[0]);
				continue;
			case 'm':	/* SGR - select graphic redition */
				TRACE(printf("= SGR>\n"));
				csi_m(cons);
				continue;

#if 0
			case 'n':	/* DSR - device status report */
				TRACE(printf("= DSR>\n"));
				if (xterm->par[0] == 5) {

					xterm_report(cons, status);

				} else {

					if (xterm->par[0] == 6) {

					     xterm_report(cons, cursor);
					}
				}
				continue;
#endif
			case '@':	/* ICH - insert (blank) character(s) */
				TRACE(printf("= ICH>\n"));
				if (! xterm->par[0]) 
					xterm->par[0]++;
				SCROLLER_INSERT_CHARS(cons->scroller, xterm->par[0]);
				continue;
#		if 0
			case 'q':	/* ??? DECLL - load LED */
					/* ??? check this! */

				if (!PAR[0]) {

					kbd_set_led_state(&KBD_STATE, 0);

				} else {

					if (PAR[0] < 5) {

						kbd_set_led(&KBD_STATE, PAR[0]-1);
						kbd_setleds(KBD);
					}
				}
				continue;
#		endif
#		ifndef STRICT_XTERM
			case 'X':
				SCROLLER_ERASE_CHARS(cons->scroller, xterm->par[0]);
				continue;
			case ']':	/* setterm functions */
				xterm_set_term(cons);
				continue;
#		endif
			}

			/*
			 * Now handle those sequences that may alter
			 * cursor position. NOTE: all cases have to
			 * be terminated with 'break'.
			 */
			SCROLLER_MODIFIED_MARK(cons->scroller);

			switch(c) {
			case 'A':	/* CUU - cursor up	*/
				TRACE(printf("= CUU>\n"));
				if (!xterm->par[0]) 
					xterm->par[0]++;

				SCROLLER_MOVE(cons->scroller, 0, - xterm->par[0]);
				break;
			case 'B':	/* CUD - cursor down	*/
			case 'e':
				TRACE(printf("= CUD>\n"));
				if (!xterm->par[0]) 
					xterm->par[0]++;

				SCROLLER_MOVE(cons->scroller, 0, xterm->par[0]);
				break;
			case 'C':	/* CUF - cursor forward	*/
			case 'a':
				TRACE(printf("= CUF>\n"));
				if (!xterm->par[0]) 
					xterm->par[0]++;

				SCROLLER_MOVE(cons->scroller, xterm->par[0], 0);
				break;
			case 'D':	/* CUB - cursor backward*/
				TRACE(printf("= CUB>\n"));
				if (!xterm->par[0]) 
					xterm->par[0]++;

				SCROLLER_MOVE(cons->scroller, - xterm->par[0], 0);
				break;
			case 'H':	/* CUP - set cursor position */
			case 'f':	/* HVP - horiz. and vert. position */
				TRACE(printf(c=='H' ? "= CUP>\n" : "= HVP>\n"));
				if (xterm->par[0]) 
					xterm->par[0]--;

				if (xterm->par[1]) 
					xterm->par[1]--;

				top = 0;
				if (CONSOLE_MODE(cons, KGI_CM_ORIGIN)) 
					SCROLLER_GET(cons->scroller, 0, &top, &bottom, 0, 0, 0, 0);

				xterm->par[0] += top;
				SCROLLER_GOTOXY(cons->scroller,
						xterm->par[1], xterm->par[0]);
				break;
			case 'L':
				TRACE(printf("= ?_?>\n"));
				if (xterm->par[0] <= 0)
					xterm->par[0] = 1;

				SCROLLER_SCROLL_TOP(cons->scroller, xterm->par[0]);
				break;
			case 'M':
				TRACE(printf("= ?_?>\n"));
				if (xterm->par[0] <= 0) 
					xterm->par[0] = 1;

				SCROLLER_SCROLL_BOTTOM(cons->scroller, xterm->par[0]);
				break;
			case 'h':	/* SM - set mode */
				TRACE(printf("= SM>\n"));
				xterm_set_mode(cons, 1);
				break;
			case 'l':	/* RM - reset mode */
				TRACE(printf("= RM>\n"));
				xterm_set_mode(cons, 0);
				break;
			case 'r':	/* DECSTBM - set top & bottom margin */
				TRACE(printf("= DECSTBM>\n"));
				SCROLLER_MARGINS(cons->scroller, xterm->par[0],
						 xterm->par[1]);
				break;
			case 'x':	/* !!! DECREQTPARM - report parameter */
				TRACE(printf("= DECREQTPARM>\n"));
				break;
#		ifndef STRICT_XTERM
			case 's':
				TRACE(printf("= save cur>\n"));
				xterm_save_cur(cons);
				break;
			case 'u':
				TRACE(printf("= rest cur>\n"));
				xterm_restore_cur(cons);
				break;
			case 'E':
				if (!xterm->par[0]) 
					xterm->par[0]++;

				SCROLLER_MOVE(cons->scroller, 0, xterm->par[0]);
				SCROLLER_GOTOX(cons->scroller, 0);
				break;
			case 'F':
				if (!xterm->par[0])
					xterm->par[0]++;
	
				SCROLLER_MOVE(cons->scroller, 0, - xterm->par[0]);
				SCROLLER_GOTOX(cons->scroller, 0);
				break;
			case 'G':
			case '`':
				if (xterm->par[0]) 
					xterm->par[0]--;

				SCROLLER_GOTOX(cons->scroller, xterm->par[0]);
				break;
			case 'd':
				if (xterm->par[0]) 
					xterm->par[0]--;

				top = 0;
				if (CONSOLE_MODE(cons, KGI_CM_ORIGIN)) {
					SCROLLER_GET(cons->scroller, 0, &top, &bottom,
						     0, 0, 0, 0);
				}
				xterm->par[0] += top;
				SCROLLER_GOTOY(cons->scroller, xterm->par[0]);
				break;
#			endif
			default:
				TRACE(printf("= unknown ESC[>\n"));
			}
			SCROLLER_MARK(cons->scroller);
			continue;
		case ESpercent:
			xterm->state = ESnormal;
			TRACE(printf("%c", (char) c));
			switch (c) {
			case '@':	/* defined in ISO 2022 */
				TRACE(printf("= UTF-8 off>\n"));
				CONSOLE_CLEAR_MODE(cons, KGI_CM_XT_UTF8);
				continue;
			case 'G':	/* prelim official escape code */
			case '8':	/* retained for compatibility */
				TRACE(printf("= UTF-8 on>\n"));
				CONSOLE_SET_MODE(cons, KGI_CM_XT_UTF8);
				continue;
			}
			TRACE(printf("= unkown ESC-percent>\n"));
			continue;
		case ESfunckey:
			xterm->state = ESnormal;
			TRACE(printf("fn-key %c\n", (char) c));
			continue;
		case EShash:
			if (c == '8') {	/* DEC screen alignment test */
				TRACE(printf("= DECSAT>\n"));
				SCROLLER_GET(cons->scroller, 0, 0, 0, 0, 0,
					     &attrfl, &erase);
				erase = (erase & 0xff00) | 'E';
				SCROLLER_SET(cons->scroller, attrfl, erase);
				SCROLLER_ERASE_DISPLAY(cons->scroller, 2);
				erase = (erase & 0xff00) | ' ';
				SCROLLER_SET(cons->scroller, attrfl, erase);
			} else {
				TRACE(printf("= unkown ESC-#>\n"));
			}
			xterm->state = ESnormal;
			continue;
		case ESset:
			TRACE(printf("%c>\n", (char) c));
			xterm_set_charset(cons, xterm->par[0], c);
			xterm->state = ESnormal;
			continue;
		default:
			TRACE(printf("<%.2x, state %i>\n", c, xterm->state));
			xterm->state = ESnormal;
			continue;
		}
	}

	if (cons->flags & KGI_CF_NEED_WRAP) 
		SCROLLER_MODIFIED_WRAP(cons->scroller);
	else
		SCROLLER_MODIFIED_MARK(cons->scroller);

	SCROLLER_SYNC(cons->scroller);

	return (n);
}

/*
 * Font translation tables.
 */

static kgi_unicode_t translation[][256] = {
  /* 8-bit Latin-1 mapped to Unicode -- trivial mapping */
#define LAT1_MAP	0
  {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
  },
  /* VT100 graphics mapped to Unicode */
#define GRAF_MAP	1
  {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x2192, 0x2190, 0x2191, 0x2193, 0x002f,
    0x2588, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x00a0,
    0x25c6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0, 0x00b1,
    0x2591, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0xf800,
    0xf801, 0x2500, 0xf803, 0xf804, 0x251c, 0x2524, 0x2534, 0x252c,
    0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
  },
  /* IBM Codepage 437 mapped to Unicode */
#define IBMPC_MAP	2
  {
    0x0000, 0x263a, 0x263b, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022, 
    0x25d8, 0x25cb, 0x25d9, 0x2642, 0x2640, 0x266a, 0x266b, 0x263c,
    0x25b6, 0x25c0, 0x2195, 0x203c, 0x00b6, 0x00a7, 0x25ac, 0x21a8,
    0x2191, 0x2193, 0x2192, 0x2190, 0x221f, 0x2194, 0x25b2, 0x25bc,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x2302,
    0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
    0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
    0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
    0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
    0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
    0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
    0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
    0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
    0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
    0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0
  }, 
  /* User mapping -- default to codes for direct font mapping */
#define USER_MAP	3
  {
    0xf000, 0xf001, 0xf002, 0xf003, 0xf004, 0xf005, 0xf006, 0xf007,
    0xf008, 0xf009, 0xf00a, 0xf00b, 0xf00c, 0xf00d, 0xf00e, 0xf00f,
    0xf010, 0xf011, 0xf012, 0xf013, 0xf014, 0xf015, 0xf016, 0xf017,
    0xf018, 0xf019, 0xf01a, 0xf01b, 0xf01c, 0xf01d, 0xf01e, 0xf01f,
    0xf020, 0xf021, 0xf022, 0xf023, 0xf024, 0xf025, 0xf026, 0xf027,
    0xf028, 0xf029, 0xf02a, 0xf02b, 0xf02c, 0xf02d, 0xf02e, 0xf02f,
    0xf030, 0xf031, 0xf032, 0xf033, 0xf034, 0xf035, 0xf036, 0xf037,
    0xf038, 0xf039, 0xf03a, 0xf03b, 0xf03c, 0xf03d, 0xf03e, 0xf03f,
    0xf040, 0xf041, 0xf042, 0xf043, 0xf044, 0xf045, 0xf046, 0xf047,
    0xf048, 0xf049, 0xf04a, 0xf04b, 0xf04c, 0xf04d, 0xf04e, 0xf04f,
    0xf050, 0xf051, 0xf052, 0xf053, 0xf054, 0xf055, 0xf056, 0xf057,
    0xf058, 0xf059, 0xf05a, 0xf05b, 0xf05c, 0xf05d, 0xf05e, 0xf05f,
    0xf060, 0xf061, 0xf062, 0xf063, 0xf064, 0xf065, 0xf066, 0xf067,
    0xf068, 0xf069, 0xf06a, 0xf06b, 0xf06c, 0xf06d, 0xf06e, 0xf06f,
    0xf070, 0xf071, 0xf072, 0xf073, 0xf074, 0xf075, 0xf076, 0xf077,
    0xf078, 0xf079, 0xf07a, 0xf07b, 0xf07c, 0xf07d, 0xf07e, 0xf07f,
    0xf080, 0xf081, 0xf082, 0xf083, 0xf084, 0xf085, 0xf086, 0xf087,
    0xf088, 0xf089, 0xf08a, 0xf08b, 0xf08c, 0xf08d, 0xf08e, 0xf08f,
    0xf090, 0xf091, 0xf092, 0xf093, 0xf094, 0xf095, 0xf096, 0xf097,
    0xf098, 0xf099, 0xf09a, 0xf09b, 0xf09c, 0xf09d, 0xf09e, 0xf09f,
    0xf0a0, 0xf0a1, 0xf0a2, 0xf0a3, 0xf0a4, 0xf0a5, 0xf0a6, 0xf0a7,
    0xf0a8, 0xf0a9, 0xf0aa, 0xf0ab, 0xf0ac, 0xf0ad, 0xf0ae, 0xf0af,
    0xf0b0, 0xf0b1, 0xf0b2, 0xf0b3, 0xf0b4, 0xf0b5, 0xf0b6, 0xf0b7,
    0xf0b8, 0xf0b9, 0xf0ba, 0xf0bb, 0xf0bc, 0xf0bd, 0xf0be, 0xf0bf,
    0xf0c0, 0xf0c1, 0xf0c2, 0xf0c3, 0xf0c4, 0xf0c5, 0xf0c6, 0xf0c7,
    0xf0c8, 0xf0c9, 0xf0ca, 0xf0cb, 0xf0cc, 0xf0cd, 0xf0ce, 0xf0cf,
    0xf0d0, 0xf0d1, 0xf0d2, 0xf0d3, 0xf0d4, 0xf0d5, 0xf0d6, 0xf0d7,
    0xf0d8, 0xf0d9, 0xf0da, 0xf0db, 0xf0dc, 0xf0dd, 0xf0de, 0xf0df,
    0xf0e0, 0xf0e1, 0xf0e2, 0xf0e3, 0xf0e4, 0xf0e5, 0xf0e6, 0xf0e7,
    0xf0e8, 0xf0e9, 0xf0ea, 0xf0eb, 0xf0ec, 0xf0ed, 0xf0ee, 0xf0ef,
    0xf0f0, 0xf0f1, 0xf0f2, 0xf0f3, 0xf0f4, 0xf0f5, 0xf0f6, 0xf0f7,
    0xf0f8, 0xf0f9, 0xf0fa, 0xf0fb, 0xf0fc, 0xf0fd, 0xf0fe, 0xf0ff
  }
#define LAST_MAP	4
};

/*
 * The standard kernel character-to-font mappings are not invertible
 * -- this is just a best effort.
 */

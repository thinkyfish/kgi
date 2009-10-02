/*-
 * Copyright (c) 1995-2000 Steffen Seeger
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
 * KGC dumb emulation.
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

#include <dev/kgc/kgc_emudumb.h>

#include "scroller_if.h"
#include "render_if.h"

/* 
 * A very dumb console parser to have output even with no parser loaded.
 */

void 
dumb_do_reset(kgi_console_t *cons, kgi_u_t do_reset)
{
	scroller_t scroll = (scroller_t)cons->scroller;
		
#if 0
	kgi_ucoord_t render_size, scroller_size;
	struct tty *tp = (struct tty *) cons->kii.tty;
	
	int s = spltty();
	
	SCROLLER_GET(cons->scroller, &scroller_size, 0, 0, 0);
	tp->t_winsize.ws_col = scroller_size.x;
	tp->t_winsize.ws_row = scroller_size.y;

	RENDER_GET(cons->render, &render_size, 0, 0);
	tp->t_winsize.ws_xpixel = render_size.x;
	tp->t_winsize.ws_ypixel = render_size.y;
	
	splx();
#endif

	cons->kii.event_mask |= KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT;

	if (do_reset) {
		SCROLLER_RESET(scroll);
		
		SCROLLER_UPDATE_ATTR(scroll);
		SCROLLER_GOTOXY(scroll, 0, 0);
		SCROLLER_ERASE_DISPLAY(scroll, 2);
	}
		
	SCROLLER_SYNC(scroll);
}


void 
dumb_handle_kii_event(kii_device_t *dev, kii_event_t *e)
{
	kgi_console_t *cons;
	struct tty *tp;
	scroller_t scroll;
	
	cons = (kgi_console_t *) dev->priv.priv_ptr;
	tp = (struct tty *)cons->kii.tty;
	scroll = (scroller_t)cons->scroller;

	if (((1 << e->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT)) ||
		(e->key.sym == K_VOID) || tp == NULL) 
		return;
	
	switch (e->key.sym & K_TYPE_MASK) {
	case K_TYPE_LATIN:
		/* Ignore non-special events if cursor hidden */
		if (CONSOLE_MODE(cons, KGI_CM_SHOW_CURSOR)) {
			tty_lock(tp);
			ttydisc_rint(tp, (char)e->key.sym, 0);
			ttydisc_rint_done(tp);
			tty_unlock(tp);
		}
		break;
	case K_TYPE_SPECIAL:
		switch (e->key.sym) {
#ifdef notyet
		case K_HOLD:
			tp->stopped ? start_tty(tp) : stop_tty(tp);
			return;
#endif
		case K_ENTER:
			if (CONSOLE_MODE(cons, KGI_CM_SHOW_CURSOR)) {
				tty_lock(tp);
				ttydisc_rint(tp, (char)ASCII_CR, 0);
				ttydisc_rint_done(tp);
				tty_unlock(tp);
			}
			break;
		case K_SCROLLFORW:
			SCROLLER_FORWARD(scroll, 0);
			SCROLLER_SYNC(scroll);
			break;
		case K_SCROLLBACK:
			SCROLLER_BACKWARD(scroll, 0);
			SCROLLER_SYNC(scroll);
			break;
		default:
			return;
		}
	}
}

int 
dumb_do_write(kgi_console_t *cons, char *buf, int count)
{
	kgi_u_t cnt;
	kgi_ascii_t c;
	scroller_t scroll;

	cnt = 0;
	scroll = (scroller_t)cons->scroller;

	SCROLLER_MARK(scroll);

	while (count) {
		c = *buf;
		buf++; cnt++; count--;

		if ((c == ASCII_LF) || (c == ASCII_CR) ||
		    (cons->flags & KGI_CF_NEED_WRAP)) {
			if (c != ASCII_CR) {
				SCROLLER_MODIFIED_MARK(scroll);
				SCROLLER_LF(scroll);
				SCROLLER_SYNC(scroll);
			}

			SCROLLER_CR(scroll);
			SCROLLER_MARK(scroll);

			if ((c == ASCII_LF) || (c == ASCII_CR))
				continue;
		}

		if (c == ASCII_BS) {
			SCROLLER_MODIFIED_MARK(scroll);
			SCROLLER_BS(scroll);
			SCROLLER_MARK(scroll);
			continue;
		}

		SCROLLER_WRITE(scroll, c);
	}

	SCROLLER_MODIFIED_MARK(scroll);
	SCROLLER_SYNC(scroll);

	return (cnt);
}

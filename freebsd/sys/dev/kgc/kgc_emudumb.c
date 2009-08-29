/*-
 * Copyright (C) 1995-2000 Steffen Seeger
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
	kgi_console_t *cons = (kgi_console_t *) dev->priv.priv_ptr;
	struct tty *tp = (struct tty *)cons->kii.tty;
	scroller_t scroll = (scroller_t)cons->scroller;
	
	if (((1 << e->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT)) ||
		(e->key.sym == K_VOID) || !tp) 
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
	kgi_u_t cnt = 0;
	kgi_ascii_t c;
	scroller_t scroll;

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

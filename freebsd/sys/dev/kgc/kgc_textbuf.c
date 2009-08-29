/*
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
 * KGI text buffer implementation
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>

/*
 * text buffer operations
 */

void 
tb_set(kgc_textbuf_t *tb, kgi_u_t src, kgi_u_t val, kgi_u_t cnt)
{
	kgi_u16_t *buf;

	buf = tb->buf + src;
	cnt--;
	do {
		buf[cnt] = val;
	} while ((kgi_s_t) --cnt >= 0);
}

void 
tb_move(kgc_textbuf_t *tb, kgi_u_t dst, kgi_u_t src, kgi_u_t cnt)
{
	char *tmp, *s;
	void *dest = tb->buf + dst;
	const void *source = tb->buf + src;
	size_t count = sizeof(kgi_u16_t)*cnt;

	if (dest <= source) {
		tmp = (char *) dest;
		s = (char *) source;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = (char *) dest + count;
		s = (char *) source + count;
		while (count--)
			*--tmp = *--s;
	}
}

#ifdef not_used

/* XXX Shall be provided by the render, not the scroller */
static void 
tb_invert(kgc_textbuf_t *tb, kgi_u_t start, kgi_u_t end)
{
	kgi_u16_t *buf;

	buf = tb->buf + start;
	while (start++ <= end) {
		*buf = ((*buf) & 0x88FF) 
			| (((*buf)<<4) & 0x7000) | (((*buf>>4)) & 0x0700);
		buf++;
	}
}

#endif /* not_used */

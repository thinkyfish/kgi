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
	kgi_u16_t *buf = tb->buf + src;

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
		tmp = (char *)dest;
		s = (char *)source;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = (char *)dest + count;
		s = (char *)source + count;
		while (count--)
			*--tmp = *--s;
	}
}

#ifdef not_used

/* 
 * XXX 
 * Shall be provided by the render, not the scroller
 */
static void 
tb_invert(kgc_textbuf_t *tb, kgi_u_t start, kgi_u_t end)
{
	kgi_u16_t *buf;

	buf = tb->buf + start;
	while (start++ <= end) {
		*buf = ((*buf) &0x88FF) 
			| (((*buf) << 4) &0x7000) | (((*buf >> 4)) &0x0700);
		buf++;
	}
}

#endif /* not_used */

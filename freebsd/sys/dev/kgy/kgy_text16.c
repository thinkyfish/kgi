/*-
 * Copyright (c) 2002 Nicholas Souchu
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
 * KGI vidsw interface display driver
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/vgareg.h>

#define	KGI_SYS_NEED_IO
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

#include <dev/kgy/kgy.h>

void dpysw_load_font(kgi_text16_t *r, kgi_u_t page, kgi_u_t size,
		kgi_u_t width, kgi_u8_t *data, kgi_u_t ch, kgi_s_t count)
{
	dpysw_display_t *sc = (dpysw_display_t *)r->meta;
	int error;

	error = (*vidsw[sc->adp->va_index]->load_font)
		(sc->adp, (int)page, (int)size, (int)width, (u_char *)data,
		(int)ch, (int)count);
	
	return;
}

void dpysw_save_font(kgi_text16_t *r, kgi_u_t page, kgi_u_t size, kgi_u_t width,
		kgi_u8_t *data, kgi_u_t ch, kgi_s_t count)
{
	dpysw_display_t *sc = (dpysw_display_t *)r->meta;
	int error;

	error = (*vidsw[sc->adp->va_index]->save_font)
		(sc->adp, (int)page, (int)size, (int)width, (u_char *)data,
		(int)ch, (int)count);
	
	return;
}

void 
dpysw_show_font(kgi_text16_t *r, kgi_u_t page)
{
	dpysw_display_t *sc = (dpysw_display_t *)r->meta;
	int error;

	error = (*vidsw[sc->adp->va_index]->show_font)
		(sc->adp, (int)page);
	
	return;
}

void 
dpysw_put_text16(kgi_text16_t *text16, kgi_u_t offset, const kgi_u16_t *text,
	  kgi_u_t count)
{
	register kgi_u16_t *fb = text16->meta_io;
	KGI_ASSERT(offset < 4096);
	KGI_ASSERT(offset + count < 4096);
	mem_put16((mem_vaddr_t) (fb + offset), text, count);
}

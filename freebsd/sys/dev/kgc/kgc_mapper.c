/*-
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
 * KGI console mapper
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MALLOC
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

kii_s_t 
kgc_unmap_kii(kii_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	if (cons && cons->render)
		RENDER_HIDE_GADGETS((render_t)cons->render);

	return (KII_EOK);
}

void 
kgc_map_kii(kii_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	if (cons && cons->render)
		RENDER_SHOW_GADGETS((render_t)cons->render, 0, 0, 0);
}

kgi_s_t 
kgc_unmap_kgi(kgi_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	if (cons && cons->scroller)
		SCROLLER_UNMAP((scroller_t)cons->scroller);
	if (cons && cons->render)
		RENDER_UNMAP((render_t)cons->render);

	return (KGI_EOK);
}

void 
kgc_map_kgi(kgi_device_t *dev)
{
	kgi_console_t *cons = dev->priv.priv_ptr;

	/* Render mapped before scroller to enable scroller sync */
	if (cons && cons->render)
		RENDER_MAP((render_t)cons->render);
	if (cons && cons->scroller)
		SCROLLER_MAP((scroller_t)cons->scroller);
}

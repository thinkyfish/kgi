/*-
 * Copyright (C) 2002 Nicholas Souchu
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
		(sc->adp, (int) page, (int)size, (int)width, (u_char *)data,
		 (int)ch, (int)count);
	
	return;
}

void dpysw_save_font(kgi_text16_t *r, kgi_u_t page, kgi_u_t size, kgi_u_t width,
			 kgi_u8_t *data, kgi_u_t ch, kgi_s_t count)
{
	dpysw_display_t *sc = (dpysw_display_t *)r->meta;
	int error;

	error = (*vidsw[sc->adp->va_index]->save_font)
		(sc->adp, (int) page, (int)size, (int)width, (u_char *)data,
		 (int)ch, (int)count);
	
	return;
}

void 
dpysw_show_font(kgi_text16_t *r, kgi_u_t page)
{
	dpysw_display_t *sc = (dpysw_display_t *)r->meta;
	int error;

	error = (*vidsw[sc->adp->va_index]->show_font)
		(sc->adp, (int) page);
	
	return;
}

void 
dpysw_put_text16(kgi_text16_t *text16, kgi_u_t offset, const kgi_u16_t *text,
	  kgi_u_t count)
{
	register kgi_u16_t *fb = text16->meta_io;
	KRN_ASSERT(offset < 4096);
	KRN_ASSERT(offset + count < 4096);
	mem_put16((mem_vaddr_t) (fb + offset), text, count);
}

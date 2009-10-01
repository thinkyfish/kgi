/*-
 * Copyright (c) 1998-2000 Steffen Seeger
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
 * KGI rendering
 */

#ifndef _kgc_kgirndr_h
#define	_kgc_kgirndr_h

#define __KGC_KGIRNDR_DATA		\
	kgi_device_t		kgi;	\
	kgi_mode_t		mode;	\
	kgi_u_t			flags;	\
	kgi_mmio_region_t	*fb;	\
	kgi_text16_t		*text16;\
	kgi_ilut_t		*ilut;	\
	kgi_tlut_t		*tlut;	\
	kgi_marker_t		*cur;	\
	kgi_marker_t		*ptr

typedef struct {
	/* Must be there if you use KGI resources interface */
	__KGC_KGIRNDR_DATA;
} kgirndr_meta;

extern kgi_u_t kgirndr_atop_color(render_t r, kgi_u_t attr);
extern kgi_u_t kgirndr_ptoa_color(render_t r, kgi_u_t val);
extern kgi_u_t kgirndr_atop_mono(kgi_u_t attr);
extern kgi_u_t kgirndr_ptoa_mono(render_t r, kgi_u_t val);
extern kgi_error_t kgirndr_init(render_t r);
extern void kgirndr_hide_gadgets(render_t r);
extern void kgirndr_undo_gadgets(render_t r);
extern void kgirndr_show_gadgets(render_t r, kgi_u_t x, kgi_u_t y, kgi_u_t offset);

#endif	/*  _kgc_kgirndr_h */

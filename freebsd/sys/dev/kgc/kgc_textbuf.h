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
 * KGI text buffer implementation
 */
#ifndef _kgc_textbuf_h
#define _kgc_textbuf_h

typedef struct {
	kgi_u16_t	*buf;	/* background/scrollback buffer	*/

	kgi_u_t		total;	/* total characters in buffer	*/
	kgi_u_t		frame;	/* characters per visible frame	*/
	kgi_ucoord_t	size;	/* visible size of buffer	*/
	kgi_ucoord_t	virt;	/* virtual size of buffer	*/

#define TBF_ALLOCATED	0x00000001
	kgi_u_t		flags;	/* flags for the buffer         */
} kgc_textbuf_t;

extern void tb_set(kgc_textbuf_t *tb, kgi_u_t src, kgi_u_t val, kgi_u_t cnt);
extern void tb_move(kgc_textbuf_t *tb, kgi_u_t dst, kgi_u_t src, kgi_u_t cnt);

#endif /* _kgc_textbuf_h */

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
 * KGI ISO font implementation
 */

#ifndef _kgc_isorndr_h
#define	_kgc_isorndr_h

/* console fonts
 *
 * The console fonts have (at maximum) 256 font positions as this can
 * (hopefully) be done or emulated on all displays. To support ISO10646 as
 * good and fast as possible, we use a (speed and common case) optimized
 * mapping not to exhaust memory:
 *
 * Generally cellinfo[ISO_ROW(x)] points to data about row ISO_ROW(x):
 *
 * * NULL if the font has no positions that map to this row.
 * * if bit number row of map_direct is set, cellinfo[row] is a pointer
 *   to an 'kgi_u8 map[256]' array, map[ISO_CELL(sym)] giving the font
 *   position. Use this if more than 31 font postions map to this row.
 * * else cellinfo[row] points to an array kgi_font_cellinfo's, sorted
 *   in ascending order of cellinfo.sym and the first giving total
 *   number of cells and highest cell assigned.
 */

typedef struct {
	kgi_isochar_t	sym;
	kgi_u_t		pos;
} kgi_console_font_cellinfo_t;

typedef struct {
	kgi_u_t		positions;	/* number of positions defined	*/
	kgi_u_t		default_pos;	/* position to use as default	*/
	__kgi_phys_addr_t map_direct[8];
	void		*cellinfo[256];	/* map info isochar->fontpos	*/
	kgi_isochar_t	inversemap[256];/* map info fontpos->isochar	*/
} kgi_console_font_info_t;

typedef struct {
	kgi_ucoord_t	size;		/* distance between char's	*/
	kgi_ucoord_t	bitmap;		/* bitmap size			*/
	kgi_u_t		base;		/* base line			*/
	kgi_console_font_info_t *info;	/* font info			*/
	kgi_u8_t	*data;		/* font data			*/
} kgi_console_font_t;

extern kgi_console_font_t *	default_font[CONFIG_KGII_MAX_NR_DEFFONTS];

#endif	/* #ifndef _kgc_isorndr_h */

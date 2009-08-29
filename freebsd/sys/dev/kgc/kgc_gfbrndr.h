/*-
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
 * KGI BSD font implementation
 */

#ifndef _kgc_gfbrndr_h
#define	_kgc_gfbrndr_h

typedef struct {
	kgi_s_t width;
	kgi_s_t height;
	kgi_u8_t data[256 * 16];
} kgc_gfb_font_t;

extern kgc_gfb_font_t kgc_bold8x16;

extern void gfbrndr_configure(kgi_console_t *cons);

#endif	/* #ifndef _kgc_gfbrndr_h */

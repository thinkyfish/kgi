/* ----------------------------------------------------------------------------
**	KGI ISO font implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**	Copyright (C)	2003		Nicholas Souchu
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ----------------------------------------------------------------------------
**
**	$FreeBSD$
**	
*/
#ifndef _kgc_isorndr_h
#define	_kgc_isorndr_h

/*	console fonts
**
**	The console fonts have (at maximum) 256 font positions as this can
**	(hopefully) be done or emulated on all displays. To support ISO10646 as
**	good and fast as possible, we use a (speed and common case) optimized
**	mapping not to exhaust memory:
**
**	Generally cellinfo[ISO_ROW(x)] points to data about row ISO_ROW(x):
**
**	  * NULL if the font has no positions that map to this row.
**	  * if bit number row of map_direct is set, cellinfo[row] is a pointer
**	    to an 'kgi_u8 map[256]' array, map[ISO_CELL(sym)] giving the font
**	    position. Use this if more than 31 font postions map to this row.
**	  * else cellinfo[row] points to an array kgi_font_cellinfo's, sorted
**	    in ascending order of cellinfo.sym and the first giving total
**	    number of cells and highest cell assigned.
*/

typedef struct
{
	kgi_isochar_t	sym;
	kgi_u_t		pos;

} kgi_console_font_cellinfo_t;

typedef struct
{
	kgi_u_t		positions;	/* number of positions defined	*/
	kgi_u_t		default_pos;	/* position to use as default	*/
	__kgi_phys_addr_t	map_direct[8];
	void		*cellinfo[256];	/* map info isochar->fontpos	*/
	kgi_isochar_t	inversemap[256];/* map info fontpos->isochar	*/

} kgi_console_font_info_t;

typedef struct
{
	kgi_ucoord_t	size;		/* distance between char's	*/
	kgi_ucoord_t	bitmap;		/* bitmap size			*/
	kgi_u_t		base;		/* base line			*/

	kgi_console_font_info_t *info;	/* font info			*/
	kgi_u8_t	*data;		/* font data			*/

} kgi_console_font_t;

extern kgi_console_font_t *	default_font[CONFIG_KGII_MAX_NR_DEFFONTS];

#endif	/* #ifndef _kgc_isorndr_h */

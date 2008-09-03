/* ----------------------------------------------------------------------------
**	KGI text buffer implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
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

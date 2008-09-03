/* ----------------------------------------------------------------------------
**	KGI rendering
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

	/* Must be there if you use KGI resources interface
	 */
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

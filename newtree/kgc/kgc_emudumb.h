/* ----------------------------------------------------------------------------
**	DUMB console parser state
** ----------------------------------------------------------------------------
**	Copyright (C)	1996-2000	Steffen Seeger
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
**/
#ifndef	_KGI_DUMB_H
#define	_KGI_DUMB_H

typedef struct
{
	kgi_console_t	cons;

} kgi_console_dumb_t;

extern void dumb_do_reset(kgi_console_t *cons, kgi_u_t do_reset);
extern void dumb_handle_kii_event(kii_device_t *dev, kii_event_t *ev);
extern int dumb_do_write(kgi_console_t *cons, char *buf, int count);

#endif	/* #ifdef _KGI_DUMB_H */

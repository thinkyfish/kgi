/* ----------------------------------------------------------------------------
**	PS/2 auxiliary device emulator
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
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
**	$Log: psaux.h,v $
*/
#ifndef _PSAUX_H
#define	_PSAUX_H

extern void psaux_init(void);
extern void psaux_handle_kii_event(kii_event_t *ev);

#endif

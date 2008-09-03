/* ----------------------------------------------------------------------------
**	KGI manager OS kernel independent stuff
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**	Copyright (C)	2002-2004	Nicholas Souchu
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
**
** ----------------------------------------------------------------------------
**	
**	$FreeBSD$
**	
*/
#ifndef _kgi_kgi_h
#define _kgi_kgi_h

/*
 * kgi.h is now just a set of includes. Each of them
 * has its own rule of #ifdef _KERNEL depending on
 * their respective purpose.
 */

#include <dev/kgi/system.h>

KGI_SYS_DECLARE_INTEGRAL_TYPES(kgi)

#include <dev/kgi/kgierr.h>
#include <dev/kgi/kgidbg.h>
#include <dev/kgi/kgityp.h>
#include <dev/kgi/kgimod.h>
#include <dev/kgi/kgires.h>
#include <dev/kgi/kgidpy.h>
#include <dev/kgi/kgicmd.h>

#endif	/* #ifndef _kgi_kgi_h */

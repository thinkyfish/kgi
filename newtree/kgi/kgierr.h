/* ----------------------------------------------------------------------------
**	KGI error codes
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
#ifndef	_kgi_kgierr_h
#define	_kgi_kgierr_h

#define KGI_EAGAIN	EAGAIN
#define KGI_EPROTO	ENOTSUP
#define KGI_ENXIO	ENXIO
#define KGI_EBADF	EBADF
#define KGI_EINVAL	EINVAL
#define KGI_ENOMEM	ENOMEM
#define KGI_EPERM	EPERM
#define KGI_EFAULT	EFAULT
#define KGI_EBUSY	EBUSY
#define KGI_ENODEV	ENODEV

enum __kgi_error
{
	KGI_EOK	= 0,
	KGI_NOMEM,
	KGI_INVAL,
	KGI_NOSUP,
	KGI_NODEV,
	KGI_FAILED,
	KGI_DLL_ERROR,
	KGI_NOT_IMPLEMENTED,
	KGI_UNKNOWN,
	KGI_CLK_LIMIT,

	KGI_NO_DISPLAY,

	KGI_LIB		= 0x01000000,
	KGI_KGI		= 0x02000000,
	KGI_DRIVER	= 0x02000000,
	KGI_MONITOR	= 0x02100000,
	KGI_RAMDAC	= 0x02200000,
	KGI_CHIPSET	= 0x02300000,
	KGI_CLOCK	= 0x02400000,
	
	KGI_DRV		= 0x03000000,

	KGI_ERROR_SUBSYSTEM = 0x7F000000,

	__KGI_LAST_ERROR

};

#define	KGI_ERRNO(system, code)	((KGI_##system) | (KGI_##code))

#endif	/* _kgi_kgierr_h */

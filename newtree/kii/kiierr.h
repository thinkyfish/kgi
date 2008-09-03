/* ----------------------------------------------------------------------------
**	KII error codes
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**	Copyright (C)	2002	Nicholas
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$FreeBSD$
**	
*/
#ifndef	_kiierr_h
#define	_kiierr_h

#define KII_EAGAIN	EAGAIN
#define KII_EPROTO	ENOTSUP
#define KII_ENXIO	ENXIO
#define KII_EBADF	EBADF
#define KII_EINVAL	EINVAL
#define KII_ENOMEM	ENOMEM
#define KII_EPERM	EPERM
#define KII_EFAULT	EFAULT
#define KII_EBUSY	EBUSY
#define KII_ENODEV	ENODEV

enum __kii_error
{
	KII_EOK	= 0,
	KII_NOMEM,
	KII_INVAL,
	KII_NOSUP,
	KII_NODEV,
	KII_FAILED,
	KII_DLL_ERROR,
	KII_NOT_IMPLEMENTED,
	KII_UNKNOWN,

	KII_NO_FOCUS,

	KII_LIB		= 0x01000000,
	KII_KII		= 0x02000000,
	KII_DRIVER	= 0x03000000,
	KII_DEVICE	= 0x03000000,

	KII_ERROR_SUBSYSTEM = 0x7F000000,

	__KII_LAST_ERROR

};

#define	KII_ERRNO(system, code)	((KII_##system) | (KII_##code))

#endif	/* #ifndef _kiierr_h	*/

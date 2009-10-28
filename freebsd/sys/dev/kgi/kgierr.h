/*-
 * Copyright (c) 1998-2000 Steffen Seeger
 * Copyright (c) 2002-2004 Nicholas Souchu
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
 * KGI error codes
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

enum __kgi_error {
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

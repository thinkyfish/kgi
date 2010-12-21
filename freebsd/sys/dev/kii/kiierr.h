/*-
 * Copyright (c) 2000 Steffen Seeger
 * Copyright (c) 2002 Nicholas
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
 * KII error codes.
 */

#ifndef	_KIIERR_H_
#define	_KIIERR_H_

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

enum __kii_error {
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

#endif /* _KIIERR_H_ */

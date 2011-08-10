/*-
 * Copyright (c) 1998-2000 Steffen Seeger
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
 * KGI kernel module library definitions
 */

#ifndef _KGIM_MODULELIB_H_
#define _KGIM_MODULELIB_H_

/*
 * debugging framework overrides
 */
#if defined(__KERNEL__) && (defined(__MODULE__) || defined(MODULE))

#undef	__krn_debug
#undef	__krn_notice
#undef	__krn_error
#ifdef	KRN_DEBUG_ANSI_CPP
#	define	__krn_debug	kgim_ansi_debug
#	define	__krn_error	kgim_ansi_error
#endif
#ifdef	KRN_DEBUG_GNU_CPP
#	define	__krn_debug	kgim_gnu_debug
#	define	__krn_error	kgim_gnu_error
#endif
#	define	__krn_notice	kgim_notice
#endif /* !defined(__KERNEL__) && defined(__MODULE__) */

extern void kgim_notice(const char *fmt, ...);

extern void kgim_ansi_debug(int level, const char *fmt, ...);
extern void kgim_ansi_error(const char *fmt, ...);

extern void kgim_gnu_debug(const char *file, int line, const char *func,
		int level, const char *fmt, ...);
extern void kgim_gnu_error(const char *file, int line, const char *func,
		const char *fmt, ...);

extern kgi_u_t kgim_attr_bits(const kgi_u8_t *bpa);

extern void	kgim_memset(void *p, kgi_u8_t val, kgi_size_t size);
extern void	kgim_memcpy(void *dst, const void *src, kgi_size_t size);
extern kgi_s_t	kgim_memcmp(const void *s1, const void *s2, kgi_size_t size);

extern kgi_s_t	kgim_strcmp(const kgi_u8_t *s1, const kgi_u8_t *s2);
extern kgi_u8_t	*kgim_strcpy(kgi_u8_t *dst, const kgi_u8_t *src);
extern kgi_u8_t	*kgim_strncpy(kgi_u8_t *dst, const kgi_u8_t *src,
			kgi_size_t size);

extern kgi_error_t kgim_display_init(kgim_display_t *dpy);
extern void kgim_display_done(kgim_display_t *dpy);

#define	KGIM_ABS(x)	(((x) < 0) ? -(x) : (x))

#endif /* !_KGIM_MODULELIB_H_ */

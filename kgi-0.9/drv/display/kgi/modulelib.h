/* ----------------------------------------------------------------------------
**	KGI kernel module library definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: modulelib.h,v $
*/
#ifndef _kgi_modulelib_h
#define _kgi_modulelib_h

/*
**	debugging framework overrides
*/
#if defined(__KERNEL__) && defined(__MODULE__)

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
#endif	/* #if defined(__KERNEL__) && defined(__MODULE__) */

extern void kgim_notice(const char *fmt, ...);

extern void kgim_ansi_debug(int level, const char *fmt, ...);
extern void kgim_ansi_error(const char *fmt, ...);

extern void kgim_gnu_debug(const char *file, int line, const char *func, int level, const char *fmt, ...);
extern void kgim_gnu_error(const char *file, int line, const char *func, const char *fmt, ...);

extern kgi_u_t kgim_attr_bits(const kgi_u8_t *bpa);

extern void	*kgim_alloc_small(kgi_u_t size);
extern void	kgim_memset(void *p, kgi_u8_t val, kgi_size_t size);
extern void	kgim_memcpy(void *dst, const void *src, kgi_size_t size);
extern kgi_s_t	kgim_memcmp(const void *s1, const void *s2, kgi_size_t size);

extern kgi_s_t	kgim_strcmp(const kgi_u8_t *s1, const kgi_u8_t *s2);
extern kgi_u8_t	*kgim_strcpy(kgi_u8_t *dst, const kgi_u8_t *src);

extern kgi_error_t kgim_display_init(kgim_display_t *dpy);
extern void kgim_display_done(kgim_display_t *dpy);

#define	KGIM_ABS(x)	(((x) < 0) ? -(x) : (x))

#endif	/* #ifdef _GGI_MODULE_H */

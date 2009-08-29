/*-
 * Copyright (C) 1998-2000 Steffen Seeger
 * Copyright (C) 2004 Nicholas Souchu
 * 
 * This file is distributed under the terms and conditions of the 
 * MIT/X public license. Please see the file COPYRIGHT.MIT included 
 * with this software for details of these terms and conditions.
 * Alternatively you may distribute this file under the terms and
 * conditions of the GNU General Public License. Please see the file 
 * COPYRIGHT.GPL included with this software for details of these terms
 * and conditions.
 */

/*
 * KGI data type definitions
 */

#ifndef	_kgi_types_h
#define	_kgi_types_h

/* XXX to deleted */
#ifdef	__STRICT_ANSI__
#	define	signed
#	define	const
#endif

typedef	int8_t		__kgi_s8_t;
typedef	uint8_t		__kgi_u8_t;

typedef	int16_t		__kgi_s16_t;
typedef	uint16_t	__kgi_u16_t;

typedef	int32_t		__kgi_s32_t;
typedef	uint32_t	__kgi_u32_t;

typedef	int64_t		__kgi_s64_t;
typedef	uint64_t	__kgi_u64_t;

typedef	int		__kgi_s_t;
typedef	unsigned int	__kgi_u_t;

typedef	char		__kgi_ascii_t;
typedef	unsigned short	__kgi_unicode_t;
typedef	unsigned int	__kgi_isochar_t;

typedef int32_t		__kgi_error_t;

typedef	vm_offset_t	__kgi_virt_addr_t;
typedef vm_paddr_t	__kgi_phys_addr_t;
typedef vm_paddr_t	__kgi_bus_addr_t;

typedef ssize_t		__kgi_size_t;

/*
 * byte order conversions
 *
 * For 8bit (byte) types, byteorder conversion doesn't make sense,
 * so only for 16,32 and 64 bit types conversion is defined.
 */

#define	HOST_BE		0
#define	HOST_LE		1

/* 
 * XXX as this is i386/types.h, i386 optimized (inline) functions should
 * XXX go here.
 */
extern __kgi_u16_t	__kgi_u16_swap(const __kgi_u16_t x);
extern __kgi_s16_t	__kgi_s16_swap(const __kgi_s16_t x);

extern __kgi_u32_t	__kgi_u32_swap(const __kgi_u32_t x);
extern __kgi_s32_t	__kgi_s32_swap(const __kgi_s32_t x);

extern __kgi_u64_t	__kgi_u64_swap(const __kgi_u64_t x);
extern __kgi_s64_t	__kgi_s64_swap(const __kgi_s64_t x);

extern __kgi_unicode_t __kgi_unicode_swap(const __kgi_unicode_t x);
extern __kgi_isochar_t __kgi_isochar_swap(const __kgi_isochar_t x);

typedef union
{
	__kgi_s8_t		priv_s8;
	__kgi_u8_t		priv_u8;

	__kgi_s16_t		priv_s16;
	__kgi_u16_t		priv_u16;

	__kgi_s32_t		priv_s32;
	__kgi_u32_t		priv_u32;

	__kgi_s64_t		priv_s64;
	__kgi_u64_t		priv_u64;

	__kgi_s_t		priv_s;
	__kgi_u_t		priv_u;

	__kgi_ascii_t		priv_ascii;
	__kgi_unicode_t		priv_unicode;
	__kgi_isochar_t		priv_isochar;

	__kgi_virt_addr_t	priv_virt_addr;
	__kgi_phys_addr_t	priv_phys_addr;
	__kgi_bus_addr_t	priv_bus_addr;

	__kgi_size_t		priv_size;

	void			*priv_ptr;
} __kgi_private_t;

#define	KGI_SYS_DECLARE_INTEGRAL_TYPES(prefix)	\
	typedef __kgi_u8_t		prefix ## _u8_t;	\
	typedef	__kgi_s8_t		prefix ## _s8_t;	\
	typedef	__kgi_u16_t		prefix ## _u16_t;	\
	typedef	__kgi_s16_t		prefix ## _s16_t;	\
	typedef	__kgi_u32_t		prefix ## _u32_t;	\
	typedef	__kgi_s32_t		prefix ## _s32_t;	\
	typedef	__kgi_u64_t		prefix ## _u64_t;	\
	typedef	__kgi_s64_t		prefix ## _s64_t;	\
	typedef	__kgi_u_t		prefix ## _u_t;		\
	typedef	__kgi_s_t		prefix ## _s_t;		\
	typedef	__kgi_error_t		prefix ## _error_t;	\
	typedef __kgi_ascii_t		prefix ## _ascii_t;	\
	typedef __kgi_unicode_t		prefix ## _unicode_t;	\
	typedef __kgi_isochar_t		prefix ## _isochar_t;	\
	typedef	__kgi_private_t		prefix ## _private_t;	\
	typedef __kgi_virt_addr_t	prefix ## _virt_addr_t;	\
	typedef __kgi_phys_addr_t	prefix ## _phys_addr_t;	\
	typedef __kgi_bus_addr_t	prefix ## _bus_addr_t;	\
	typedef __kgi_size_t		prefix ## _size_t;

#endif	/* #ifdef _kgi_types_h */

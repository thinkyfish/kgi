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
#ifndef _kgi_kgityp_h
#define _kgi_kgityp_h

#define	KB	*1024
#define	MB	*(1024*1024)
#define	GB	*(1024*1024*1024)


/*	ISO 10646 defines symbol codes as 32 bit unsigned int organized into
**	groups, planes, rows and cells. By now only the first plane (group=0,
**	plane=0) is defined, which maps directly to UNICODE. Use these macros
**	to extract group, plane, row or cell.
*/
#define KGI_ISOCHAR_GROUP(isochar)	(((isochar) >> 24) & 0xFF)
#define	KGI_ISOCHAR_PLANE(isochar)	(((isochar) >> 16) & 0xFF)
#define	KGI_ISOCHAR_ROW(isochar)	(((isochar) >>  8) & 0xFF)
#define	KGI_ISOCHAR_CELL(isochar)	((isochar) & 0xFF)

typedef struct { kgi_u8_t major, minor, patch, extra; } kgi_version_t;
typedef struct { kgi_s_t x,y; }		kgi_scoord_t;
typedef struct { kgi_u_t x,y; }		kgi_ucoord_t;
typedef struct { kgi_u8_t x,y; }	kgi_u8_coord_t;
typedef struct { kgi_s_t min, max; }	kgi_srange_t;
typedef struct { kgi_u_t min, max; }	kgi_urange_t;

typedef struct { kgi_u_t mul, div; }	kgi_ratio_t;

typedef struct { kgi_u16_t r,g,b; }	kgi_rgb_color_t;
typedef union {

	kgi_rgb_color_t	rgb;

} kgi_color_t;

/*
**	byte order conversions
*/
typedef enum 
{
	KGI_ENDIAN_LITTLE,
	KGI_ENDIAN_BIG
} kgi_endian_t;

#define	KGI_MAX_NR_FOCUSES	CONFIG_KGII_MAX_NR_FOCUSES
#define	KGI_MAX_NR_CONSOLES	CONFIG_KGII_MAX_NR_CONSOLES
#define	KGI_MAX_NR_DEVICES	CONFIG_KGII_MAX_NR_DEVICES
#define	KGI_MAX_NR_DISPLAYS	CONFIG_KGII_MAX_NR_DISPLAYS

#define	KGI_VALID_FOCUS_ID(x)	((x) < KGI_MAX_NR_FOCUSES)
#define	KGI_VALID_CONSOLE_ID(x)	((x) < KGI_MAX_NR_CONSOLES)
#define	KGI_VALID_DEVICE_ID(x)	((x) < KGI_MAX_NR_DEVICES)
#define	KGI_VALID_DISPLAY_ID(x)	((x) < KGI_MAX_NR_DISPLAYS)

#define	KGI_INVALID_FOCUS	INVALID_FOCUS
#define	KGI_INVALID_CONSOLE	INVALID_CONSOLE
#define	KGI_INVALID_DISPLAY	INVALID_DISPLAY
#define	KGI_INVALID_DEVICE	INVALID_DEVICE

/*
 * Preliminary definition to extend type visibility
 */

#define	__KGI_MAX_NR_RESOURCES		16
#define	__KGI_MAX_NR_IMAGE_RESOURCES	16
#define __KGI_MAX_NR_ATTRIBUTES		32

typedef struct kgi_resource_s kgi_resource_t;

#endif /* _kgi_kgityp_h */

/* ----------------------------------------------------------------------------
**	KGI console default font data structures
** ----------------------------------------------------------------------------
**	Copyright (C)	1996-2000	Steffen Seeger
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
**	$FreeBSD$
**	
*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <dev/kgi/opt_kgi.h>

#include <dev/kgi/system.h>
#include <dev/kgi/kgi.h>
#include <dev/kgi/kgii.h>

#include <dev/kgc/kgc_isorndr.h>

#define	CONFIG_ISO_FONT_8x8
#define	CONFIG_ISO_FONT_8x14
#define	CONFIG_ISO_FONT_9x16

#ifndef CONFIG_ISO_FONT_FILE
#define CONFIG_ISO_FONT_FILE <kgc/default-font-ibm437>
#endif
#include CONFIG_ISO_FONT_FILE

kgi_console_font_t *default_font[CONFIG_KGII_MAX_NR_DEFFONTS] =
{	/* !!! FIXME: for now there must be only one font per height*/
	CONFIG_ISO_FONT_9x16
	CONFIG_ISO_FONT_8x14
	CONFIG_ISO_FONT_8x8
	NULL
};

#ifdef notyet
#include <kgi/default-ptr>
#include <kgi/default-color>
#endif /* notyet */

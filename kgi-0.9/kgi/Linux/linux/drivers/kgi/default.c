/* ----------------------------------------------------------------------------
**	default data structures
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
**	$Log: default.c,v $
**	Revision 1.2  2001/07/03 08:48:41  seeger_s
**	- added default pointer shape and color lookup table
**	
**	Revision 1.1.1.1  2000/04/18 08:50:47  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#include <linux/config.h>
#include <linux/types.h>
#include <linux/kgii.h>
#include <kii/kii.h>
#include <kgi/kgi.h>
#include "console.h"

#define	CONFIG_KGII_FONT_8x8
#define	CONFIG_KGII_FONT_8x14
#define	CONFIG_KGII_FONT_9x16
#define	CONFIG_KGII_FONT_FILE	"default-font-ibm437"
#include CONFIG_KGII_FONT_FILE

kgi_console_font_t *default_font[CONFIG_KGII_MAX_NR_DEFFONTS] =
{	/* !!! FIXME: for now there must be only one font per height*/
	CONFIG_KGII_FONT_9x16
	CONFIG_KGII_FONT_8x14
	CONFIG_KGII_FONT_8x8
	NULL
};

#define	CONFIG_KGII_KEYMAP	"default-keymap-de"
/* #define	CONFIG_KGII_KEYMAP	"default-keymap-us" */
#include CONFIG_KGII_KEYMAP

#include "default-ptr"
#include "default-color"

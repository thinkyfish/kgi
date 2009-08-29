/*-
 * Copyright (C) 1996-2000 Steffen Seeger
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
 * default keymap data structures
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kii/kii.h>

#ifndef CONFIG_KGII_KEYMAP
/* #define	CONFIG_KGII_KEYMAP	"default-keymap-de" */
#define	CONFIG_KGII_KEYMAP	<dev/kip/default-keymap-us>
#endif
#include CONFIG_KGII_KEYMAP

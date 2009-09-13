/*-
 * Copyright (C) 1998-2000 Steffen Seeger
 * Copyright (C) 2002-2004 Nicholas Souchu
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
 * KGI manager OS kernel independent foo.
 */

#ifndef _kgi_kgii_h
#define _kgi_kgii_h

/*
 * KGI/KII limits. 
 * NOTE	All limits need to be less than 255.
 */
#define	CONFIG_KGII_MAX_NR_DISPLAYS	8
#define	CONFIG_KGII_MAX_NR_INPUTS	255
#define	CONFIG_KGII_MAX_NR_FOCUSES	8
#define	CONFIG_KGII_MAX_NR_CONSOLES	96
#define	CONFIG_KGII_MAX_NR_DEVICES	(2*CONFIG_KGII_MAX_NR_CONSOLES)

#define	INVALID_DISPLAY	((unsigned)-1)
#define	INVALID_INPUT	((unsigned)-1)
#define	INVALID_FOCUS	((unsigned)-1)
#define	INVALID_CONSOLE	((unsigned)-1)
#define	INVALID_DEVICE	((unsigned)-1)

#define	CONFIG_KGII_MAX_NR_DEFFONTS	4
#define	CONFIG_KGII_CONSOLEBUFSIZE	(16*PAGE_SIZE)

#define	CONFIG_KGI_DISPLAYS	4

/*
 * Public API
 */
extern int dpy_null_init(int display, int max_display);
extern void kgi_init(void);
extern int focus_init(void);
extern int kii_configure(int flags);

/*
 * Public data, kgi.c 
 */
extern unsigned char console_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
extern unsigned char graphic_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
extern unsigned char focus_map[CONFIG_KGII_MAX_NR_DEVICES];
extern unsigned char display_map[CONFIG_KGII_MAX_NR_DEVICES];

#endif	/* _kgi_kgii_h */

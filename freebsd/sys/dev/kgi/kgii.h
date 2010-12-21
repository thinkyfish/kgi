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
 * KGI manager OS kernel independent foo.
 */

#ifndef _KGI_KGII_H_
#define _KGI_KGII_H_

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

#endif	/* _KGI_KGII_H_ */

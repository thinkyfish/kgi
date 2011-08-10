/*-
 * Copyright (c) 1996-2000 Steffen Seeger
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
 * KGI console default font data structures
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

kgi_console_font_t *default_font[CONFIG_KGII_MAX_NR_DEFFONTS] = {
	/* XXX FIXME: for now there must be only one font per height*/
	CONFIG_ISO_FONT_9x16
	CONFIG_ISO_FONT_8x14
	CONFIG_ISO_FONT_8x8
	NULL
};

#ifdef notyet
#include <kgi/default-ptr>
#include <kgi/default-color>
#endif /* notyet */

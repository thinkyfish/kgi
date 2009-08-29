/*-
 * Copyright (c) 1995-1998 Søren Schmidt
 * All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Sascha Wildner <saw@online.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * From FreeBSD: /head/sys/dev/syscons/syscons.h r189617
 */

#ifndef _DEV_SYSCONS_SYSCONS_H_
#define	_DEV_SYSCONS_SYSCONS_H_

#include <sys/lock.h>
#include <sys/mutex.h>

/* machine-dependent part of the header */

#ifdef PC98
#include <pc98/cbus/sc_machdep.h>
#elif defined(__i386__)
/* nothing for the moment */
#endif

/* default values for configuration options */

#ifndef MAXCONS
#define MAXCONS		24
#endif

#ifdef SC_NO_SYSMOUSE
#undef SC_NO_CUTPASTE
#define SC_NO_CUTPASTE	1
#endif

#ifdef SC_NO_MODE_CHANGE
#undef SC_PIXEL_MODE
#endif

/* Always load font data if the pixel (raster text) mode is to be used. */
#ifdef SC_PIXEL_MODE
#undef SC_NO_FONT_LOADING
#endif

/* 
 * If font data is not available, the `arrow'-shaped mouse cursor cannot
 * be drawn.  Use the alternative drawing method.
 */
#ifdef SC_NO_FONT_LOADING
#undef SC_ALT_MOUSE_IMAGE
#define SC_ALT_MOUSE_IMAGE 1
#endif

#ifndef SC_CURSOR_CHAR
#define SC_CURSOR_CHAR	(0x07)
#endif

#ifndef SC_MOUSE_CHAR
#define SC_MOUSE_CHAR	(0xd0)
#endif

#if SC_MOUSE_CHAR <= SC_CURSOR_CHAR && SC_CURSOR_CHAR < (SC_MOUSE_CHAR + 4)
#undef SC_CURSOR_CHAR
#define SC_CURSOR_CHAR	(SC_MOUSE_CHAR + 4)
#endif

#ifndef SC_DEBUG_LEVEL
#define SC_DEBUG_LEVEL	0
#endif

#define DPRINTF(l, p)	if (SC_DEBUG_LEVEL >= (l)) printf p

#ifndef __sparc64__
#define SC_DRIVER_NAME	"sce"
#else
/*
 * Use a different driver name on sparc64 so it does not get confused
 * with the system controller devices which are also termed 'sc' in OFW.
 */
#define SC_DRIVER_NAME	"sce"
#endif
#define SC_VTY(dev)	(((sc_ttysoftc *)tty_softc(tp))->st_index)
#define SC_DEV(sc, vty)	((sc)->dev[(vty) - (sc)->first_vty])
#define SC_STAT(tp)	(*((scr_stat **)&((sc_ttysoftc *)tty_softc(tp))->st_stat))

/* printable chars */
#ifndef PRINTABLE
#define PRINTABLE(ch)	((ch) > 0x1b || ((ch) > 0x0d && (ch) < 0x1b) \
			 || (ch) < 0x07)
#endif

/* macros for "intelligent" screen update */
#define mark_for_update(scp, x)	{\
			  	    if ((x) < scp->start) scp->start = (x);\
				    else if ((x) > scp->end) scp->end = (x);\
				}
#define mark_all(scp)		{\
				    scp->start = 0;\
				    scp->end = scp->xsize * scp->ysize - 1;\
				}

/* vty status flags (scp->status) */
#define UNKNOWN_MODE	0x00010		/* unknown video mode */
#define SWITCH_WAIT_REL	0x00080		/* waiting for vty release */
#define SWITCH_WAIT_ACQ	0x00100		/* waiting for vty ack */
#define BUFFER_SAVED	0x00200		/* vty buffer is saved */
#define CURSOR_ENABLED 	0x00400		/* text cursor is enabled */
#define MOUSE_MOVED	0x01000		/* mouse cursor has moved */
#define MOUSE_CUTTING	0x02000		/* mouse cursor is cutting text */
#define MOUSE_VISIBLE	0x04000		/* mouse cursor is showing */
#define GRAPHICS_MODE	0x08000		/* vty is in a graphics mode */
#define PIXEL_MODE	0x10000		/* vty is in a raster text mode */
#define SAVER_RUNNING	0x20000		/* screen saver is running */
#define VR_CURSOR_BLINK	0x40000		/* blinking text cursor */
#define VR_CURSOR_ON	0x80000		/* text cursor is on */
#define MOUSE_HIDDEN	0x100000	/* mouse cursor is temporarily hidden */

/* misc defines */
#define FALSE		0
#define TRUE		1

/*
   The following #defines are hard-coded for a maximum text
   resolution corresponding to a maximum framebuffer
   resolution of 1600x1200 with an 8x8 font...
*/
#define	COL		200
#define	ROW		150

#define PCBURST		128

typedef struct {
	union {	/* keep it first */
		kgi_console_t any;
		kgi_console_dumb_t dumb;
		kgi_console_xterm_t xterm;
	} type;
} sce_console;

typedef struct sce_ttysoftc {
	int unit;
	//mutex;
} sce_ttysoftc;

extern sce_console *sce_consoles[CONFIG_KGII_MAX_NR_CONSOLES];

extern int sce_mouse_init(void);
extern int sce_sysmouse_init(void);
extern void sce_sysmouse_event(kii_event_t *ev);

#endif /* !_DEV_SYSCONS_SYSCONS_H_ */

/*-
 * Copyright (c) 2003 Nicholas Souchu
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
 * KGI FreeBSD boot console.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"
#include "opt_syscons.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	1
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/consio.h>
#include <sys/fbio.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <dev/fb/fbreg.h>

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kii/kii.h>
#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_scroller.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_gfbrndr.h>
#include <dev/kgc/kgc_emuxterm.h>
#include <dev/kgc/kgc_emudumb.h>
#include <dev/sce/sce_syscons.h>

#include "scroller_if.h"
#include "render_if.h"

/* Define here for early init without allocation. */
static sce_console scecons;
static kgi_u16_t sce_buf[CONFIG_KGII_CONSOLEBUFSIZE];

/* Prototypes */
static void console_printk(char *b, unsigned count);
static void do_reset(kgi_console_t *cons);
static int event2char(kii_device_t *dev, kii_event_t *ev);
static void sce_handle_kii_event(kii_device_t *dev, kii_event_t *ev);

/* Console */
static cn_probe_t	sce_cnprobe;
static cn_init_t	sce_cninit;
static cn_term_t	sce_cnterm;
static cn_getc_t	sce_cngetc;
static cn_putc_t	sce_cnputc;

static const struct consdev_ops sce_cnops = {
	.cn_probe = sce_cnprobe,
	.cn_init  = sce_cninit,
	.cn_term  = sce_cnterm,
	.cn_getc  = sce_cngetc,
	.cn_putc  = sce_cnputc
};

static struct consdev sce_cndev = {
	.cn_ops  = &sce_cnops
};

/*
 * This is a printk() implementation based on the scroll->* functions. 
 */
static void 
console_printk(char *b, unsigned count)
{
	kgi_console_t *cons;
	kgi_u_t c, attr;
	static int printing = 0;

	cons = (kgi_console_t *)&scecons;

	if (printing || cons == NULL) 
		return;

	printing = 1;

	SCROLLER_MARK(cons->scroller);
	SCROLLER_GET(cons->scroller, 0, 0, 0, 0, 0, &attr, 0);

	while (count--) {
		c = *(b++);

		/* Is it a printable character? */
		if (c >= 32) {			
			 /* Have kernel/boot console messages stand out. */
			attr |= KGI_CA_BOLD;	
			SCROLLER_SET(cons->scroller, attr, 0);
			SCROLLER_UPDATE_ATTR(cons->scroller);

			if (cons->flags & KGI_CF_NEED_WRAP) {
				SCROLLER_MODIFIED_MARK(cons->scroller);
				if (CONSOLE_MODE(cons, KGI_CM_AUTO_WRAP)) {
					SCROLLER_LF(cons->scroller);
				}
				SCROLLER_CR(cons->scroller);
				SCROLLER_MARK(cons->scroller);
			}

			SCROLLER_WRITE(cons->scroller, c);

		} else {
			/* Remove bold. */
			attr &= ~KGI_CA_BOLD;
			SCROLLER_SET(cons->scroller, attr, 0);
			SCROLLER_UPDATE_ATTR(cons->scroller);

			switch (c) {
			case ASCII_LF:
				SCROLLER_MODIFIED_MARK(cons->scroller);
				SCROLLER_LF(cons->scroller);
				SCROLLER_MARK(cons->scroller);
				/* FALL THROUGH */
			case ASCII_CR:
				SCROLLER_MODIFIED_MARK(cons->scroller);
				SCROLLER_CR(cons->scroller);
				SCROLLER_MARK(cons->scroller);
				break;
			case ASCII_BS:
				SCROLLER_MODIFIED_MARK(cons->scroller);
				SCROLLER_BS(cons->scroller);
				SCROLLER_MARK(cons->scroller);
				break;
			default:
				break;
			}
		}
	}

	if (cons->flags & KGI_CF_NEED_WRAP) 
		SCROLLER_MODIFIED_WRAP(cons->scroller);
	else 
		SCROLLER_MODIFIED_MARK(cons->scroller);
	
	SCROLLER_SYNC(cons->scroller);
	printing = 0;
}

static void 
do_reset(kgi_console_t *cons)
{
	scroller_t scroll;
	
	scroll = (scroller_t)cons->scroller;	
	cons->kii.event_mask = KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT | 
			KII_EM_POINTER;

	SCROLLER_RESET(scroll);

	SCROLLER_UPDATE_ATTR(scroll);
	SCROLLER_GOTOXY(scroll, 0, 0);
	SCROLLER_ERASE_DISPLAY(scroll, 2);

	SCROLLER_SYNC(scroll);
}

static int 
event2char(kii_device_t *dev, kii_event_t *ev)
{
	kgi_console_t *cons;
	int s;

	cons = (kgi_console_t *)dev->priv.priv_ptr;
	/* Forward to sysmouse pointer events. */
 	if ((1 << ev->any.type) & KII_EM_POINTER) {
 		sce_sysmouse_event(ev);
 		return (-1);
 	}
	
	if (((1 << ev->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT)) ||
		(ev->key.sym == K_VOID)) {
		return (-1);
	}

	s = spltty();
	switch (ev->key.sym & K_TYPE_MASK) {
	case K_TYPE_LATIN:
		return (ev->key.sym & K_VALUE_MASK);
	case K_TYPE_SPECIAL:
		switch (ev->key.sym) {
		case K_ENTER:
			return ('\n');
			break;
		case K_SCROLLFORW:
			SCROLLER_FORWARD(cons->scroller, 0);
			SCROLLER_SYNC(cons->scroller);
			break;
		case K_SCROLLBACK:
			SCROLLER_BACKWARD(cons->scroller, 0);
			SCROLLER_SYNC(cons->scroller);
			break;
		default:
			break;
		}
	}
	splx(s);

	return (-1);
}

static void 
sce_handle_kii_event(kii_device_t *dev, kii_event_t *ev)
{
	int discard;

	discard = event2char(dev, ev);
}

/*
 * Console probe.
 */
static void
sce_cnprobe(struct consdev *cp)
{

	cp->cn_pri = CN_INTERNAL;	
	strcpy(cp->cn_name, "ttyv0");
}

/*
 * Console init
 * Setup up the console with KGI's renderer, input interface & scroller.
 */
static void
sce_cninit(struct consdev *cp)
{
	kgi_console_t *cons = (kgi_console_t *)&scecons;

	memset(cons, 0, sizeof(*cons));

	/* 
	 * Use backdoors at early initializations.
	 */
	textscroller_configure(cons);
	gfbrndr_configure(cons);

	/* Reserve the first cons as boot console. */
	sce_consoles[0] = &scecons;

	cons->kii.flags |= KII_DF_CONSOLE;
	cons->kii.priv.priv_ptr = cons;
	cons->kii.HandleEvent	= &sce_handle_kii_event;

	cons->meta_console = (void *)cp;

	/* 
	 * Init the renderer for the KGI device registered to device 0
	 */
	if (RENDER_INIT((render_t)cons->render, 0))   
		panic("Could not init renderer!");

	if (kii_register_device(&(cons->kii), 0))  
		panic("Could not register input!");

	if (SCROLLER_INIT((scroller_t)cons->scroller, sce_buf)) 
		panic("Could not reset scroller state!");

	cons->refcnt++;

	kii_map_device(cons->kii.id);
	
	do_reset(cons);	
}

/*
 * Console finish
 */
static void
sce_cnterm(struct consdev *cp)
{
}

/*
 * Console get char
 */
static int
sce_cngetc(struct consdev *cp)
{
	int c, s;
	kii_event_t e;

	s = spltty();
	for (;;) {
		kii_poll_device(0, &e);
		c = event2char(&scecons.type.any.kii, &e);
		if (c != -1)
			break;
	}
	splx(s);
	return (c);
}

/*
 * Console put char
 */
static void
sce_cnputc(struct consdev *cp, int c)
{
	char cc;

 	cc = (char)c;
	console_printk(&cc, 1);
}

static void
scecn_init(void)
{

	/*
	 * Access the video adapter driver through the back door!
	 * Video adapter drivers need to be configured before syscons.
	 * However, when syscons is being probed as the low-level console,
	 * they have not been initialized yet.  We force them to initialize
	 * themselves here.
	 */
	vid_configure(VIO_PROBE_ONLY);
}
SYSINIT(sce, SI_SUB_KGI, SI_ORDER_ANY, scecn_init, NULL);

static int
scecn_mod_event(module_t mod, int type, void *data)
{
 
 	switch (type) {
 	case MOD_LOAD:
 		memset(sce_consoles, 0, sizeof(sce_consoles));	
 		sce_cnprobe(&sce_cndev);	
 		sce_cninit(&sce_cndev);
 		cnadd(&sce_cndev);
 		break;
 	case MOD_UNLOAD:
 		return (ENXIO);
 	default:
 		break;
 	}
 
 	return (0);
}

static moduledata_t scecn_mod = {
	"scecn",
	scecn_mod_event,
	NULL,
};

/* SI_ORDER_ANY to init after VESA if present */
DECLARE_MODULE(scecn, scecn_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_VERSION(scecn, 1);

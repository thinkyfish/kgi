/* ----------------------------------------------------------------------------
**	KGI FreeBSD boot console
** ----------------------------------------------------------------------------
**	Copyright (C)	2003		Nicholas Souchu
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

#include <dev/scemul/sce_syscons.h>

#include "scroller_if.h"
#include "render_if.h"

#define CDEV_MAJOR	12

static cn_probe_t	sce_cn_probe;
static cn_init_t	sce_cn_init;
static cn_term_t	sce_cn_term;
static cn_getc_t	sce_cn_getc;
static cn_checkc_t	sce_cn_checkc;
static cn_putc_t	sce_cn_putc;

#ifdef notyet
CONS_DRIVER(sce, sce_cn_probe, sce_cn_init, sce_cn_term, sce_cn_getc,
	    sce_cn_checkc, sce_cn_putc, NULL);
#else
/* Define manually until we activate the CONS_DRIVER macro */
static struct consdev sce_consdev = {
	sce_cn_probe, sce_cn_init, sce_cn_term, sce_cn_getc,
	sce_cn_checkc, sce_cn_putc, NULL };	

#endif

/* Define here for early init without allocation */
static sce_console_t scecons;
static kgi_u16_t sce_buf[CONFIG_KGII_CONSOLEBUFSIZE];

/*
**	This is a printk() implementation based on the scroll->* functions. 
*/

static void console_printk(char *b, unsigned count)
{
	kgi_console_t *cons = (kgi_console_t *)&scecons;
	kgi_u_t c;
	static int printing = 0;

	if (printing || !cons) {

		return;
	}

	printing = 1;

	SCROLLER_MARK(cons->scroller);

	while (count--) {

		c = *(b++);

		/* Is it a printable character? */
		if (c >= 32) {

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

	if (cons->flags & KGI_CF_NEED_WRAP) {

		SCROLLER_MODIFIED_WRAP(cons->scroller);
	} else {
		SCROLLER_MODIFIED_MARK(cons->scroller);
	}

	SCROLLER_SYNC(cons->scroller);
	printing = 0;
}

static void do_reset(kgi_console_t *cons)
{
	scroller_t scroll = (scroller_t)cons->scroller;
	cons->kii.event_mask = KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT | KII_EM_POINTER;

	SCROLLER_RESET(scroll);

	SCROLLER_UPDATE_ATTR(scroll);
	SCROLLER_GOTOXY(scroll, 0, 0);
	SCROLLER_ERASE_DISPLAY(scroll, 2);

	SCROLLER_SYNC(scroll);
}

static int event2char(kii_device_t *dev, kii_event_t *ev)
{
	kgi_console_t *cons = (kgi_console_t *)dev->priv.priv_ptr;
	int s;

	/* Forward to sysmouse pointer events */
	if ((1 << ev->any.type) & KII_EM_POINTER) {
		sce_sysmouse_event(ev);
		return -1;
	}
	
	if (((1 << ev->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT)) ||
		(ev->key.sym == K_VOID)) {
		return -1;
	}

	s = spltty();
	switch (ev->key.sym & K_TYPE_MASK) {

	case K_TYPE_LATIN:
		return (ev->key.sym & K_VALUE_MASK);

	case K_TYPE_SPECIAL:
		switch (ev->key.sym) {

		case K_ENTER:
			return '\n';
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

	return -1;
}

static void handle_kii_event(kii_device_t *dev, kii_event_t *ev)
{
	int discard;

	discard = event2char(dev, ev);
}

/*---------------------------------------------------------------------------*
 *	console probe
 *---------------------------------------------------------------------------*/
static void
sce_cn_probe(struct consdev *cp)
{
	int unit = 0;
	int i;

	/* sce stands for the internal console of the system.
	 */
	cp->cn_pri = CN_INTERNAL;

	/* See if this driver is disabled in probe hint. */ 
	if (resource_int_value("sce", unit, "disabled", &i) == 0 && i)
		cp->cn_pri = CN_DEAD;

	if (cp->cn_pri == CN_DEAD)
		return;

	sprintf(cp->cn_name, "ttyv%r", 0);
}

/*---------------------------------------------------------------------------*
 *	console init
 *---------------------------------------------------------------------------*/
static void
sce_cn_init(struct consdev *cp)
{
	kgi_console_t *cons = (kgi_console_t *)&scecons;

	memset(cons, 0, sizeof(*cons));

	/* Use backdoors at early initializations
	 */
	textscroller_configure(cons);
	gfbrndr_configure(cons);

	/* Reserve the first cons as boot console */
	sce_consoles[0] = &scecons;

	cons->kii.flags |= KII_DF_CONSOLE;
	cons->kii.priv.priv_ptr = cons;
	cons->kii.HandleEvent	= &handle_kii_event;

	cons->meta_console = (void *)cp;

	/* Init the renderer for have the KGI device registered
	 * to device 0
	 */
	if (RENDER_INIT((render_t)cons->render)) {
	  
		panic("Could not init render!");
	}

	if (kii_register_device(&(cons->kii), 0)) {
	  
		panic("Could not register input!");
	}

	if (SCROLLER_INIT((scroller_t)cons->scroller, sce_buf)) {

		panic("Could not reset scroller state!");
	}
	cons->refcnt++;

	kii_map_device(cons->kii.id);
	
	do_reset(cons);
	
	console_initialized = 1;
}

/*---------------------------------------------------------------------------*
 *	console finish
 *---------------------------------------------------------------------------*/
static void
sce_cn_term(struct consdev *cp)
{
	return;
}

/*---------------------------------------------------------------------------*
 *	console put char
 *---------------------------------------------------------------------------*/
static void
sce_cn_putc(struct consdev *cd, int c)
{
	char cc = (char) c;

	console_printk(&cc, 1);
}

/*---------------------------------------------------------------------------*
 *	console check for char
 *---------------------------------------------------------------------------*/
static int
sce_cn_checkc(struct consdev *cd)
{
	kii_event_t event;
	int s, c;

	s = spltty();

	kii_poll_device(0, &event);
	c = event2char(&scecons.type.any.kii, &event);

	splx(s);

	return (c);
}

/*---------------------------------------------------------------------------*
 *	console get char
 *---------------------------------------------------------------------------*/
static int
sce_cn_getc(struct consdev *cd)
{
	int c;

	while ((c = sce_cn_checkc(cd)) == -1)
		;

	return (c);
}

/* Early initialization of sce. Configure the video drivers.
 */
static void
sce_cn_vid_init(void)
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

SYSINIT(sce, SI_SUB_KGI, SI_ORDER_MIDDLE, sce_cn_vid_init, NULL);

static int
scecn_mod_event(module_t mod, int type, void *data)
{
	switch (type) {
	case MOD_LOAD:
		memset(sce_consoles, 0, sizeof(sce_consoles));
		
		sce_cn_probe(&sce_consdev);
		sce_cn_init(&sce_consdev);

		cnadd(&sce_consdev);
		break;

	case MOD_UNLOAD:
		return ENXIO;

	default:
		break;
	}

	return 0;
}

static moduledata_t scecn_mod = {
	"scecn",
	scecn_mod_event,
	NULL,
};

/* SI_ORDER_ANY to init after VESA if present */
DECLARE_MODULE(scecn, scecn_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_VERSION(scecn, 1);

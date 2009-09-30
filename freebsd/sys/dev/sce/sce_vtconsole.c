/*-
 * Copyright (C) 1995-2000 Steffen Seeger
 * Copyright (C) 2003 Nicholas Souchu
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
  * KGI console driver.
  */
  
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

/* 
 * XXX FIXME 
 * I think this FIXME may be referring to the use of compile time terminal
 * emulator selection?
 */
#define KGC_TERM_XTERM

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	4
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/callout.h>
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/priv.h>
#include <sys/random.h>
#include <sys/signalvar.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/kbio.h>

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

static int first_minor_allocated = 0;
sce_console *sce_consoles[CONFIG_KGII_MAX_NR_CONSOLES];

/* Prototypes */
static void assign_parser(kgi_console_t *cons, int do_reset);
static void sce_handle_kii_event(kii_device_t *dev, kii_event_t *e);
static int sce_close_vt(int unit);
static int sce_ttytounit(struct tty *tp);
static int sce_init_vt(struct tty *tp);
static struct tty* sce_create_tty(int unit);

/* TTY */
static tsw_ioctl_t		sce_tswioctl;
static tsw_open_t		sce_tswopen;
static tsw_outwakeup_t	sce_tswoutwakeup;

static struct ttydevsw scevt_ttydevsw = {
	.tsw_ioctl		= sce_tswioctl,
	.tsw_open		= sce_tswopen,
	.tsw_outwakeup	= sce_tswoutwakeup
};

static d_ioctl_t scevt_ioctl;

static struct cdevsw scevt_devsw = {
	.d_version	= D_VERSION,
	.d_flags	= D_NEEDGIANT,
	.d_ioctl	= scevt_ioctl,
	.d_name		= "scevt"
};

/*
 * XXX
 * This should handle assignment of registered terminal emulators later.
 * For now we only enter the neccessary fields for the dumb or xterm parser.
 */
static void 
assign_parser(kgi_console_t *cons, int do_reset)
{

	KRN_ASSERT(cons);

	cons->kii.MapDevice		= kgc_map_kii;
	cons->kii.UnmapDevice	= kgc_unmap_kii;
	cons->kii.HandleEvent	= &sce_handle_kii_event;
	cons->kii.event_mask	= KII_EM_POINTER;
#ifdef KGC_TERM_XTERM
	cons->DoWrite = &xterm_do_write;
	xterm_do_reset(cons, do_reset);
#else
#ifndef KGC_TERM_DUMB
#error KGC_TERM_DUMB not defined
#endif
	cons->DoWrite = &dumb_do_write;
	dumb_do_reset(cons, do_reset);
#endif
}

static void 
sce_handle_kii_event(kii_device_t *dev, kii_event_t *e)
{

	/* Forward to sysmouse pointer events */
 	if ((1 << e->any.type) & KII_EM_POINTER) {
 		sce_sysmouse_event(e);
 		return;
 	}
#ifdef KGC_TERM_XTERM
	xterm_handle_kii_event(dev, e);
#else
#ifndef KGC_TERM_DUMB
#error KGC_TERM_DUMB not defined
#endif
	dumb_handle_kii_event(dev, e);
#endif
}

/*
 * Create a console & associate a TTY with it.
 */
static int
sce_init_vt(struct tty *tp)
{	
	kgi_u_t unit;
	kgi_console_t *cons;
	kgi_error_t err;

	unit = sce_ttytounit(tp);
	if (unit >= CONFIG_KGII_MAX_NR_CONSOLES) {
		KRN_ERROR("Reached maximum amount of consoles.");
		return (ENXIO);
	}

	cons = (kgi_console_t *)sce_consoles[unit];

	if (cons == NULL) {
		KRN_DEBUG(3, "Creating virtual terminal %d", unit);
		cons = kgi_kmalloc(sizeof(sce_console));
		if (cons == NULL) {
			KRN_ERROR("Failed: Not enough memory.");
			return (ENOMEM);
		}
		memset(cons, 0, sizeof(sce_console));

		/* If the minor is null, the console was not initialized. */
		if (unit == 0)
			first_minor_allocated = 1;
	}	
	
	cons->kii.tty = tp;
	cons->kii.flags |= KII_DF_CONSOLE;
	cons->kii.priv.priv_ptr = cons;

	/*
	 * Check if console is setup & attach KGI's scroller and renderer
	 * classes to it.
	 */
	if (sce_consoles[unit] == NULL) {
		err = kii_register_device(&(cons->kii), unit);
		if (err != KII_EOK) {
			KRN_ERROR("Failed: Could not register input on %d %d", unit, err);
			goto fail_reg_device;
		}

		/*
		 * Allocate a new render instance based on the render class
		 * registered to our display.
		 */
		cons->render = kgc_render_alloc(unit, NULL);
		if (cons->render == NULL) {			
			KRN_ERROR("Failed: Could not allocate render device %d", unit);
			goto fail_render_alloc;
		}

		((render_t) cons->render)->cons = cons;	/* XXX */
		if (RENDER_INIT((render_t)cons->render, 0)) {			
			KRN_ERROR("Failed: Could not initialize renderer!");
			goto fail_render_init;
		}

		/*
		 * Allocate a new scroller instance based on the scroller class
		 * registered to our display.
		 */
		cons->scroller = kgc_scroller_alloc(unit, NULL);
		if (cons->scroller == NULL) {			
			KRN_ERROR("Failed: Could not allocate scroller device %d", unit);
			goto fail_scroller_alloc;
		}

		((render_t)cons->scroller)->cons = cons; /* XXX */		
		if (SCROLLER_INIT((scroller_t)cons->scroller, NULL)) {
			KRN_ERROR("Failed: Could not reset console");
			goto fail_scroller_init;
		}

		if (kii_current_focus(cons->kii.focus_id) == NULL)
			kii_map_device(cons->kii.id);

		/*
		 * Initialization OK.
		 */
		sce_consoles[unit] = (sce_console *)cons;
		KRN_DEBUG(4, "Virtual terminal console %i allocated.", unit);
	}

	assign_parser(cons, (unit) ? 1 : 0);

	return (KGI_EOK);

 fail_scroller_init: /* Fall thru. */
	kgc_scroller_release(unit);
 fail_scroller_alloc: /* Fall thru. */
	RENDER_DONE((render_t)cons->render);
 fail_render_init: /* Fall thru. */
	kgc_render_release(unit);
 fail_render_alloc: /* Fall thru. */
	kii_unregister_device(&(cons->kii));
 fail_reg_device:
	if ((cons && unit) || (cons && first_minor_allocated)) {
		kgi_kfree(cons);
		sce_consoles[unit] = NULL;
		first_minor_allocated = 0;
	}
	
	return (ENXIO);	
}

/*
 * Cleanup and remove virtual terminals.
 */
static int
sce_close_vt(int unit)
{
	kgi_console_t *cons;
	
	cons = (kgi_console_t *)sce_consoles[unit];

	unit = sce_ttytounit(cons->kii.tty);
	if (unit >= CONFIG_KGII_MAX_NR_CONSOLES) {
		KRN_ERROR("Bad console %i", unit);
		return (EINVAL);
	}
	
	if (cons && (unit || first_minor_allocated)) {
		KRN_DEBUG(2, "Freeing console %i", unit);

		if (cons->kii.flags & KII_DF_FOCUSED)
			kii_unmap_device(cons->kii.id);

		kii_unregister_device(&cons->kii);

		SCROLLER_DONE((scroller_t)cons->scroller);
		cons->scroller = NULL;
		kgc_scroller_release(unit);

		RENDER_DONE((render_t)cons->render);
		cons->render = NULL;
		kgc_render_release(unit);

		sce_consoles[unit] = NULL;
		kgi_kfree(cons);

		if ((unit == 0) && first_minor_allocated)
			first_minor_allocated = 0;
	}
	return (KGI_EOK);
}

/*
 * Get unit number of TTY device.
 */
static inline int 
sce_ttytounit(struct tty *tp)
{

	return (((sce_ttysoftc* )tty_softc(tp))->unit);
}

/*
 * Allocate and create TTY device.
 */
static struct tty *
sce_create_tty(int unit)
{
	struct tty *tp;
	struct sce_ttysoftc *sc; /* Used to store device unit. */
	
	/* Allocate ttysoftc. */
	sc = malloc(sizeof(sce_ttysoftc), M_DEVBUF, M_WAITOK);
	sc->unit = unit;	
	
	/* Allocate TTY & store device number. */
	KRN_DEBUG(5, "Allocating TTY %i", unit);
	tp = tty_alloc(&scevt_ttydevsw, sc);
	
	/* Create TTY device node. */
	tty_makedev(tp, NULL, "v%r", unit);
	
	return (tp);
}

static int
sce_tswioctl(struct tty *tp, u_long cmd, caddr_t data, struct thread *td)
{

	switch (cmd) {
		/* Translate from KII to KBD format. */
	case GIO_KEYMAP: /* Get keyboard translation table. */					 
	case PIO_KEYMAP: /* Set keyboard translation table. */
	case GIO_KEYMAPENT:	/* Get keyboard translation table entry. */
	case PIO_KEYMAPENT:	/* Set keyboard translation table entry. */
	case GIO_DEADKEYMAP: /* Get accent key translation table. */
	case PIO_DEADKEYMAP: /* Set accent key translation table. */
	case GETFKEY:		 /* Get functionkey string. */
	case SETFKEY:		 /* Set functionkey string. */
		return (ENOTTY);
	default:
		break;
	}

	/* Leave ioctl up to the TTY system. */
	return (ENOIOCTL);
}

/* 
 * TTY open routine. 
 */
static int
sce_tswopen(struct tty *tp)
{
	int unit;
	kgi_ucoord_t sz, rz;
	kgi_console_t *cons;

	/* 
	 * Set the window dimensions of the TTY if they're not already set.
	 */
	unit = sce_ttytounit(tp);
	cons = (kgi_console_t *)sce_consoles[unit];
	if (cons == NULL)
		return (ENXIO);

	if (tp->t_winsize.ws_col == 0 || tp->t_winsize.ws_row == 0) {
		SCROLLER_GET(cons->scroller, &sz, 0, 0, 0, 0, 0, 0);
		RENDER_GET(cons->render, &rz, 0, 0);

		tp->t_winsize.ws_col = sz.x;
		tp->t_winsize.ws_xpixel = rz.x;	
		tp->t_winsize.ws_row = sz.y;
		tp->t_winsize.ws_ypixel = rz.y;
	}

	return (0);
}

/*
 * Receive data from the TTY system and pass it to the KGC terminal layer.
 */
static void 
sce_tswoutwakeup(struct tty *tp)
{
 	int unit;
	kgi_console_t *cons;
	size_t len;
	u_char buf[PCBURST];

	unit = sce_ttytounit(tp);
	cons = (kgi_console_t *)sce_consoles[unit];
	if (cons == NULL)
		return;

 	kiidev_sync(&(cons->kii), KII_SYNC_LED_FLAGS);
	
	KRN_DEBUG(8, "Receiving data from TTY %d", unit);

	for (;;) {
		/* Fill the buffer. */
		len = ttydisc_getc(tp, buf, sizeof(buf));
		KRN_DEBUG(8, "%d bytes in buffer.", len);
		if (len == 0)
			break;
		cons->DoWrite(cons, buf, len);
	}	
}

static int
scevt_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag,
		struct thread *td)
{

	return (sce_tswioctl(dev->si_drv1, cmd, data, td));
}

/*
 * Load the syscons emulator virtual console.
 */
static int
scevt_mod_event(module_t mod, int type, void *data)
{
	int err, unit;
	struct cdev *dev;
	struct tty *tp;
	static int scevt_init = SCEVT_COLD;

	switch (type) {
	case MOD_LOAD:
		/* 
		 * XXX 
		 * MAXCONS used here should be auto at kbd plug?
		 */
		if (scevt_init == SCEVT_COLD ) {
			for (unit = 0; unit < MAXCONS; unit++) {
				tp = sce_create_tty(unit);
				sce_init_vt(tp);
			}
			dev = make_dev(&scevt_devsw, 0, UID_ROOT, GID_WHEEL, 0600, "scevt");
			scevt_init = SCEVT_WARM;
#ifndef SC_NO_SYSMOUSE
			sce_mouse_init();
			sce_sysmouse_init();
#endif
		}		
		return (0);
	case MOD_UNLOAD:
		if (scevt_init == SCEVT_WARM) {
			for (unit = 0; unit < MAXCONS; unit++) {
				err = sce_close_vt(unit);
				if (err != KGI_EOK)
					KRN_ERROR("Failed to remove console %d", unit);
			}
			scevt_init = SCEVT_COLD;
		}
		return (ENXIO);
	default:
		break;
	}

	return (0);
}

static moduledata_t scevt_mod = {
	"scevt",
	scevt_mod_event,
	NULL,
};

DECLARE_MODULE(scevt, scevt_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_VERSION(scevt, 1);


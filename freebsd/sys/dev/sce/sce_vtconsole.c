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
  * KGI console driver
  */
  
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

/* XXX FIXME */
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
//tsw_ioctl_t *sce_tsw_ioctl;

/* Prototypes */
static void handle_kii_event(kii_device_t *dev, kii_event_t *e);
static void assign_parser(kgi_console_t *cons, int do_reset);
static int sce_init_vt(struct tty *tp);
static struct tty* sce_init_tty(int unit);
static int sce_dev2unit(struct tty *tp);

/* TTY */
static tsw_close_t		sce_vtclose;
static tsw_open_t		sce_vtopen;
//static tsw_ioctl_t		sce_vtioctl;
static tsw_outwakeup_t	sce_vtoutwakeup;

static struct ttydevsw scevt_ttydevsw = {
//	.tsw_flags		= TF_OPENED_CONS,
	.tsw_close		= sce_vtclose,
	.tsw_open		= sce_vtopen,
//	.tsw_ioctl		= sce_vtioctl,
	.tsw_outwakeup	= sce_vtoutwakeup
};

static struct cdevsw scevt_devsw = {
	.d_version	= D_VERSION,
	.d_flags	= D_NEEDGIANT,
	.d_name		= "scevt"
};

static void 
handle_kii_event(kii_device_t *dev, kii_event_t *e)
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
	cons->kii.HandleEvent	= &handle_kii_event;
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

/*
 * Register KII with a TTY & assign a render + scroller
 * to create a virtual terminal.
 */
static int
sce_init_vt(struct tty *tp)
{	
	kgi_u_t unit;
	kgi_console_t *cons;
	kgi_error_t err;

	unit = sce_dev2unit(tp);
	if (unit >= MAXCONS) {
		KRN_ERROR("Reached maximum amount of consoles.");
		return (ENXIO);
	}

	KRN_DEBUG(2, "Creating VT %d", unit);
	cons = (kgi_console_t *)sce_consoles[unit];

	if (!cons) {
		KRN_DEBUG(2, "Allocating console %d...", unit);

		cons = kgi_kmalloc(sizeof(sce_console));
		if (!cons) {
			KRN_ERROR("Failed: Not enough memory.");
			return (ENOMEM);
		}
		memset(cons, 0, sizeof(sce_console));

		/* If the minor is null, the console was not initialized */
		if (unit == 0)
			first_minor_allocated = 1;
	}	
	
	cons->kii.tty = tp;
	cons->kii.flags |= KII_DF_CONSOLE;
	cons->kii.priv.priv_ptr = cons;

	/*
	 * Check if console is setup.
	 */
	if (!sce_consoles[unit]) {
		if ((err = kii_register_device(&(cons->kii), unit))) {
			KRN_ERROR("Failed: Could not register input on %d %d", unit, err);
			goto failed1;
		}

		/*
		 * Allocate a new render instance based on the render class
		 * registered to our display.
		 */
		if (!(cons->render = kgc_render_alloc(unit, NULL))) {			
			KRN_ERROR("Failed: Could not allocate render device %d", unit);
			goto failed2;
		}
		((render_t) cons->render)->cons = cons;			/* XXX */
		if (RENDER_INIT((render_t)cons->render, 0)) {			
			KRN_ERROR("Failed: Could not init render!");
			goto failed3;
		}

		/*
		 * Allocate a new scroller instance based on the scroller class
		 * registered to our display.
		 */
		if (!(cons->scroller = kgc_scroller_alloc(unit, NULL))) {			
			KRN_ERROR("Failed: Could not allocate scroller device %d", unit);
			goto failed4;
		}
		((render_t)cons->scroller)->cons = cons;		/* XXX */		
		if (SCROLLER_INIT((scroller_t)cons->scroller, NULL)) {
			KRN_ERROR("Failed: Could not reset console");
			goto failed5;
		}

		/*
		 * Initialization OK. Set console now from working copy (cons).
		 */
		sce_consoles[unit] = (sce_console *)cons;

		if (!kii_current_focus(cons->kii.focus_id))
			kii_map_device(cons->kii.id);

		KRN_DEBUG(4, "VT Console %i allocated.", unit);
	}

	assign_parser(cons, (unit) ? 1 : 0);

	return (KGI_EOK);

 failed5: /* Fall thru. */
	kgc_scroller_release(unit);
 failed4: /* Fall thru. */
	RENDER_DONE((render_t)cons->render);
 failed3: /* Fall thru. */
	kgc_render_release(unit);
 failed2: /* Fall thru. */
	kii_unregister_device(&(cons->kii));
 failed1:
	if ((cons && unit) || (cons && first_minor_allocated)) {
		kgi_kfree(cons);
		sce_consoles[unit] = NULL;
		first_minor_allocated = 0;
	}
	
	return (ENXIO);	
}

/*
 * Allocate and create TTY device.
 */
static struct tty *
sce_init_tty(int unit)
{
	struct tty *tp;
	struct sce_ttysoftc *sc; /* Used to store device unit. */
	
	/* Allocate ttysoftc. */
	sc = malloc(sizeof(struct sce_ttysoftc), M_DEVBUF, M_WAITOK);
	sc->unit = unit;	
	
	/* Allocate TTY & store device number. */
	KRN_DEBUG(5, "Allocating TTY %i", unit);
	tp = tty_alloc(&scevt_ttydevsw, sc);
	
	/* Create TTY device node. */
	tty_makedev(tp, NULL, "v%r", unit);
	
	return (tp);
}

/*
 * Get unit number of TTY device.
 */
static int 
sce_dev2unit(struct tty *tp)
{
	int unit;
	
	unit = ((sce_ttysoftc* )tty_softc(tp))->unit;

	return (unit);	
}

static int 
sce_vtparam(struct tty *tp, struct termios *t)
{

	tp->t_termios.c_ispeed = t->c_ispeed;
	tp->t_termios.c_ospeed = t->c_ospeed;
	tp->t_termios.c_cflag  = t->c_cflag;

	return (0);
}

static void 
sce_vtoutwakeup(struct tty *tp)
{
	int s;
	int unit;
	kgi_console_t *cons;
	size_t len;
	u_char buf[PCBURST];

	unit = sce_dev2unit(tp);
	cons = (kgi_console_t *)sce_consoles[unit];
	if (!cons)
		return;

/*	cons->kii.flags |= KII_DF_SCROLL_LOCK; */
	s = spltty();
	kiidev_sync(&(cons->kii), KII_SYNC_LED_FLAGS);

	if (tp->t_flags & (TF_BUSY | TF_STOPPED))
		goto out;

	tp->t_flags |= TF_BUSY;
	splx(s);
	
	for (;;) {
		len = ttydisc_getc(tp, buf, sizeof(buf));
		if (len == 0);
			break;
		cons->DoWrite(cons, buf, len);
	}

	tp->t_flags &= ~TF_BUSY;
	tty_wakeup(tp, 0);
	return;

out:
	splx(s);
	return;
}

static void
sce_vtclose(struct tty *tp)
{
	kgi_u_t unit;
	kgi_console_t *cons;
	
	unit = sce_dev2unit(tp);
	if (unit >= CONFIG_KGII_MAX_NR_CONSOLES) {
		KRN_ERROR("Bad console %i", unit);
		return; //(EINVAL);
	}

	cons = (kgi_console_t *)sce_consoles[unit];

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
}

static int
sce_vtopen(struct tty *tp)
{
	
	if (!tty_opened(tp)) {
		tp->t_termios.c_iflag  = TTYDEF_IFLAG;
		tp->t_termios.c_oflag  = TTYDEF_OFLAG;
		tp->t_termios.c_cflag  = TTYDEF_CFLAG;
		tp->t_termios.c_lflag  = TTYDEF_LFLAG;
		tp->t_termios.c_ispeed = tp->t_termios.c_ospeed = TTYDEF_SPEED;
		sce_vtparam(tp, &tp->t_termios);
		ttydisc_modem(tp, 1);
	} else if (tp->t_flags & TF_EXCLUDE)
		return (EBUSY);
	
	return (KGI_EOK);
}

// static int
// sce_vtioctl(struct tty *tp, u_long cmd, caddr_t data, struct thread *td)
// {
// 	int error;
// 
// 	//error = (*sce_tsw_ioctl)(tp, cmd, data, td);
// 	error = tty_ioctl(tp, cmd, data, td);
// 	if (error != ENOIOCTL)
// 		return (error);
// 
// 	switch (cmd) {
// 	case GIO_KEYMAP:	/* get keyboard translation table */
// 		/* Translate from KII to KBD format */
// 		break;
// 	case PIO_KEYMAP:	/* set keyboard translation table */
// #ifndef KBD_DISABLE_KEYMAP_LOAD
// #endif
// 		break;
// 	case GIO_KEYMAPENT:	/* get keyboard translation table entry */
// 		break;
// 	case PIO_KEYMAPENT:	/* set keyboard translation table entry */
// #ifndef KBD_DISABLE_KEYMAP_LOAD
// #endif
// 		break;
// 	case GIO_DEADKEYMAP:	/* get accent key translation table */
// 		break;
// 	case PIO_DEADKEYMAP:	/* set accent key translation table */
// #ifndef KBD_DISABLE_KEYMAP_LOAD
// #endif
// 		break;
// 	case GETFKEY:		/* get functionkey string */
// 		break;
// 	case SETFKEY:		/* set functionkey string */
// #ifndef KBD_DISABLE_KEYMAP_LOAD
// #endif
// 		break;
// 	default:
// 		break;
// 	}
// 
// 	return (ENOTTY);
// }

/*
 * Load the syscons emulator virtual console.
 */
static int
sce_vt_mod_event(module_t mod, int type, void *data)
{
	int unit;
	struct cdev *dev;
	struct tty *tp;

	switch (type) {
	case MOD_LOAD:
		/* 
		 * XXX 
		 * MAXCONS should be auto at kbd plug?
		 */
		for(unit = 0; unit < MAXCONS; unit++) {
			tp = sce_init_tty(unit);
			sce_init_vt(tp);
		}
		dev = make_dev(&scevt_devsw, 0, UID_ROOT, GID_WHEEL, 0600, "scevt");
#ifndef SC_NO_SYSMOUSE
		sce_mouse_init();
		sce_sysmouse_init();
#endif
		return (0);
	case MOD_UNLOAD:
		return (ENXIO);
	default:
		break;
	}

	return (0);
}

static moduledata_t scevt_mod = {
	"scevt",
	sce_vt_mod_event,
	NULL,
};

DECLARE_MODULE(scevt, scevt_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_VERSION(scevt, 1);


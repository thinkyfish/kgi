/*-
 * Copyright (C) 1998-2000 Steffen Seeger
 * Copyright (C) 2002 Nicholas Souchu
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
 * KII console input manager
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	1
#endif

#define	KGI_SYS_NEED_IO
#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_PROC
#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

static int vt_dont_switch = 0;	/* XXX should be in vt.c */

/*
 * Kernel input actions.
 */

static void 
make_sound(kii_focus_t *f, kii_u_t frequency, kii_u_t duration){
	KRN_DEBUG(2, "make_sound() not implemented yet!");
}

/*
 * This routine runs with all interrupts enabled and does all the things
 * that may have to be done in response to a handled event, but may take
 * a reasonably long time.
 */
void 
kii_bottomhalf(void)
{
	kii_u_t i;

	for (i = 0; i < KII_MAX_NR_FOCUSES; i++) {
		kii_s_t dev;
		kii_focus_t *f;

		if (!(f = kiifocus[i]))
			continue;

		if (vt_dont_switch)
			goto ignore;

		/* 
		 * If the focus has no dev or what we have differs from what
		 * we want (console vs graphic), then force switch.
		 */
		if (!f->focus)
			goto apply;

		if (!((f->flags & KII_FF_PROCESS_BH) &&
		      KII_VALID_DEVICE_ID(f->want_console) &&
		      (f->want_console != f->focus->id)))
			goto ignore;

	apply:	
		if (KII_VALID_CONSOLE_ID(f->want_console))
			dev = f->console_map[f->want_console];
		else
			dev = f->graphic_map[f->want_console - KII_MAX_NR_CONSOLES];
		
		/*
		 * Only check for kiidevice not wanting to introduce
		 * on kgidevice. If the kgi device doesn't exist
		 * unmap will fail anyway.
		 */
		if (!(kiidevice[dev])) {
			/* 
			 * Converted from a KRN_ERROR to KRN_DEBUG
			 * to avoid verbosity with syscons.
			 */
			KRN_DEBUG(2, "Invalid device %i (console %i)", 
				  dev, f->want_console);
			f->want_console = KII_INVALID_CONSOLE;
			continue;
		}

		f->flags &= ~KII_FF_PROCESS_BH;

		switch (kii_unmap_device(dev)) {
		case KII_EINVAL:
			f->want_console = KII_INVALID_CONSOLE;
		case KII_EAGAIN:
			continue;
		case KII_EOK:
			break;
		default:
			KRN_INTERNAL_ERROR;
		}

		/* 
		 * Don't bother about KGI device, allow usage of /dev/event alone.
		 */
		kgi_unmap_device(dev);
#ifdef dont_bother
		switch (kgi_unmap_device(dev)) {
		case KII_EINVAL:
			f->want_console = KII_INVALID_CONSOLE;
		case KII_EAGAIN:
			KRN_DEBUG(2, "Could not unmap kgi device %i", dev);

			/* Map at least the input to let a chance to blind commands. */
			kii_map_device(f->curr_console);
			continue;
		case KII_EOK:
			break;
		default:
			KRN_INTERNAL_ERROR;
		}
#endif

		kii_map_device(dev);
		kgi_map_device(dev);

		f->last_console = f->curr_console;
		f->curr_console = f->want_console;

	ignore:
		f->want_console = KII_INVALID_CONSOLE;
	}
}

static void 
do_bottomhalf(void)
{
	
	/* Wakeup the KGI daemon to process the event. */
	kgi_wakeup(&kgiproc);
}

static void 
do_special(kii_focus_t *f, kii_event_t *event)
{
	u_int curdev;

	if (event->any.type != KII_EV_KEY_PRESS) 
		return;

	KRN_DEBUG(3, "Doing special key sym=%.4x code=%d effect=%d", event->key.sym,
		  event->key.code, event->key.effect);

	switch(event->key.sym) {
	case K_VOID:
	case K_ENTER:
		return;
	case K_CONS:
		if (!KII_VALID_CONSOLE_ID(f->want_console) &&
			KII_VALID_CONSOLE_ID(f->last_console)) {
			f->want_console = f->last_console;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	case K_CAPS:
		f->flags ^= KII_FF_CAPS_SHIFT;
		return;
	case K_BOOT:		
		if ((curdev = kgi_current_devid(0)) != -1) {
			kgi_unmap_device(curdev);
		}
		kgi_map_device(0);

		shutdown_nice(0);

		/* NOT REACHED */
	case K_CAPSON:
		f->flags |= KII_FF_CAPS_SHIFT;
		return;
	case K_COMPOSE:
		f->dead = K_COMPOSE;
		return;
	case K_DECRCONSOLE: {
		register kii_s_t start, search;

		if (!KII_VALID_CONSOLE_ID(f->curr_console)) {
			search = (start = KII_MAX_NR_CONSOLES - 1) - 1;
		} else {
			search = ((start = f->curr_console) > 0)
				? f->curr_console - 1 : KII_MAX_NR_CONSOLES - 1;
		}

		while ((search != start) && !(
			(KII_VALID_DEVICE_ID(f->console_map[search]) &&
				kiidevice[f->console_map[search]]) ||
			(KII_VALID_DEVICE_ID(f->graphic_map[search]) &&
				kiidevice[f->graphic_map[search]]))) {

			if (--search <  0) 
				search = KII_MAX_NR_CONSOLES - 1;			
		}
		if ((search != start) && 
			!KII_VALID_CONSOLE_ID(f->want_console)) {
			f->want_console = search;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	}
	case K_INCRCONSOLE: {
		register kii_s_t start, search;

		if (! KII_VALID_CONSOLE_ID(f->curr_console)) {
			search = (start = 0) + 1;
		} else {
			KRN_ASSERT(KII_VALID_CONSOLE_ID(f->curr_console));
			search = ((start = f->curr_console) <
				KII_MAX_NR_DEVICES - 1) 
				? f->curr_console + 1 : 0;
		}

		while ((search != start) && !(
			(KII_VALID_DEVICE_ID(f->console_map[search]) &&
				kiidevice[f->console_map[search]]) ||
			(KII_VALID_DEVICE_ID(f->graphic_map[search]) &&
				kiidevice[f->graphic_map[search]]))) {

			if (++search >= KII_MAX_NR_CONSOLES) 
				search = 0;
		}

		if ((search != start) &&
			!KII_VALID_CONSOLE_ID(f->want_console)) {
			f->want_console = search;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	}
	case K_SPAWNCONSOLE:
		KRN_ASSERT(sizeof(pid_t) <= sizeof(kii_u_t));
		if (f->focus && ((pid_t) f->focus->spawnpid.priv_u)) {

#ifdef notavail	/* XXX */
			if (kill_proc(((pid_t) f->focus->spawnpid.priv_u), 
				f->focus->spawnsig.priv_u, 1)) {

				f->focus->spawnpid.priv_u = 0;
			}
#endif
		}
		event->any.type = KII_EV_NOTHING;
		return;
	case K_TOGGLESCREEN:
		if (KII_VALID_DEVICE_ID(f->curr_console)) {
			f->want_console = f->curr_console;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	case K_SYSTEM_REQUEST:

		f->flags |= KII_FF_SYSTEM_REQUEST;
		
		if ((curdev = kgi_current_devid(0)) != -1) {
			kgi_unmap_device(curdev);
		}
		kgi_map_device(0);

		kdb_enter(KDB_WHY_UNSET, "KII escape to debugger.");

		if (curdev) {
			kgi_unmap_device(0);
			kgi_map_device(curdev);
		}

		f->flags &= ~KII_FF_SYSTEM_REQUEST;

		return;
	default:
		return;
	}

	/* NOT REACHED */
}

void 
kii_action(kii_focus_t *f, kii_event_t *event)
{
	kii_u_t sym = event->key.sym;

	if ((1 << event->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_RELEASE)) 
		return;	

	KRN_DEBUG(3, "Key %s, code 0x%.2x, sym %.2x", 
		(event->key.type == KII_EV_KEY_PRESS) ? "down" : "up",
		event->key.code, event->key.sym);

	switch (event->key.sym & K_TYPE_MASK) {
	case K_TYPE_SPECIAL:
		if (sym < K_LAST_SPECIAL) {
			do_special(f, event); 
		}
		return;
	case K_TYPE_CONSOLE:
		sym -= K_FIRST_CONSOLE;
		if (KII_VALID_DEVICE_ID(sym) &&
			(event->key.type == KII_EV_KEY_PRESS)) {
			if (KII_VALID_CONSOLE_ID(f->want_console)) {
				if (f->want_console == sym) {
					f->flags |= KII_FF_PROCESS_BH;
					do_bottomhalf();
				} else {
					make_sound(f, 444, 250);
					/* BEEP! */
				}
			} else {
				f->want_console = sym;
				f->flags |= KII_FF_PROCESS_BH;
				do_bottomhalf();
			}
		}
		return;
	}
}

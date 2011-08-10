/*-
 * Copyright (c) 1998-2000 Steffen Seeger
 * Copyright (c) 2002 Nicholas Souchu
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
 * KII console input manager
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#include <sys/param.h>
#include <sys/systm.h>

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
make_sound(kii_focus_t *f, kii_u_t frequency, kii_u_t duration)
{

	KGI_DEBUG(5, "make_sound(%p, %i, %i)", f, frequency, duration);
	sysbeep(frequency, duration);
}

/*
 * This routine runs with all interrupts enabled and does all the things
 * that may have to be done in response to a handled event, but may take
 * a reasonably long time.
 */
void
kii_bottomhalf(void)
{
	kii_s_t dev;
	kii_u_t i;
	kii_focus_t *f;

	for (i = 0; i < KII_MAX_NR_FOCUSES; i++) {
		f = kiifocus[i];
		if (f == NULL)
			continue;

		if (vt_dont_switch)
			goto ignore;

		/*
		 * If the focus has no dev or what we have differs from what
		 * we want (console vs graphic), then force switch.
		 */
		if (f->focus == NULL)
			goto apply;

		if (((f->flags & KII_FF_PROCESS_BH) &&
		    KII_VALID_DEVICE_ID(f->want_console) &&
		    (f->want_console != f->focus->id)) == 0)
			goto ignore;

	apply:
		if (KII_VALID_CONSOLE_ID(f->want_console))
			dev = f->console_map[f->want_console];
		else {
			dev = f->graphic_map[f->want_console
				- KII_MAX_NR_CONSOLES];
		}
		/*
		 * Only check for kiidevice not wanting to introduce
		 * on kgidevice. If the kgi device doesn't exist
		 * unmap will fail anyway.
		 */
		if (kiidevice[dev] == NULL) {
			/*
			 * Converted from a KGI_ERROR to KGI_DEBUG
			 * to avoid verbosity with syscons.
			 */
			KGI_DEBUG(13, "Invalid device %i (console %i)",
				  dev, f->want_console);
			f->want_console = KII_INVALID_CONSOLE;
			continue;
		}

		f->flags &= ~KII_FF_PROCESS_BH;

		switch (kii_unmap_device(dev)) {
		case KII_EINVAL:
			f->want_console = KII_INVALID_CONSOLE;
			/* Fall thru. */
		case KII_EAGAIN:
			continue;
		case KII_EOK:
			break;
		default:
			KGI_INTERNAL_ERROR;
		}

		/*
		 * Don't bother about KGI device, allow usage of /dev/event
		 * alone.
		 */
		kgi_unmap_device(dev);
#ifdef dont_bother
		switch (kgi_unmap_device(dev)) {
		case KII_EINVAL:
			f->want_console = KII_INVALID_CONSOLE;
			/* Fall thru. */
		case KII_EAGAIN:
			KGI_DEBUG(2, "Could not unmap kgi device %i", dev);
			/*
			 * Map at least the input to let a chance to blind
			 * commands.
			 */
			kii_map_device(f->curr_console);
			continue;
		case KII_EOK:
			break;
		default:
			KGI_INTERNAL_ERROR;
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

	KGI_DEBUG(10, "Doing special key sym=%.4x code=%d effect=%d",
		  event->key.sym, event->key.code, event->key.effect);

	switch(event->key.sym) {
	case K_VOID: /* Fall thru. */
	case K_ENTER:
		return;
	case K_CONS:
		if (KII_VALID_CONSOLE_ID(f->want_console) &&
		    KII_VALID_CONSOLE_ID(f->last_console) == 0) {
		    f->want_console = f->last_console;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	case K_CAPS:
		f->flags ^= KII_FF_CAPS_SHIFT;
		return;
	case K_BOOT:
		curdev = kgi_current_devid(0);
		if (curdev != -1)
			kgi_unmap_device(curdev);

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

		if (KII_VALID_CONSOLE_ID(f->curr_console) == 0)
			search = (start = KII_MAX_NR_CONSOLES - 1) - 1;
		else {
			search = ((start = f->curr_console) > 0) ?
				f->curr_console - 1 : KII_MAX_NR_CONSOLES - 1;
		}

		while ((search != start) &&
		    ((KII_VALID_DEVICE_ID(f->console_map[search]) &&
		    kiidevice[f->console_map[search]]) ||
		    (KII_VALID_DEVICE_ID(f->graphic_map[search]) &&
		    kiidevice[f->graphic_map[search]])) == 0) {
			if (--search <  0)
				search = KII_MAX_NR_CONSOLES - 1;
		}
		if ((search != start) &&
		    KII_VALID_CONSOLE_ID(f->want_console) == 0) {
			f->want_console = search;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	}
	case K_INCRCONSOLE: {
		register kii_s_t start, search;

		if (KII_VALID_CONSOLE_ID(f->curr_console) == 0) {
			search = (start = 0) + 1;
		} else {
			KGI_ASSERT(KII_VALID_CONSOLE_ID(f->curr_console));
			search = ((start = f->curr_console) <
			    KII_MAX_NR_DEVICES - 1) ? f->curr_console + 1 : 0;
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
		    KII_VALID_CONSOLE_ID(f->want_console) == 0) {
			f->want_console = search;
			f->flags |= KII_FF_PROCESS_BH;
			do_bottomhalf();
		}
		return;
	}
	case K_SPAWNCONSOLE:
		KGI_ASSERT(sizeof(pid_t) <= sizeof(kii_u_t));
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

		curdev = kgi_current_devid(0);
		if (curdev != -1)
			kgi_unmap_device(curdev);

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
	kii_u_t sym;

	if ((1 << event->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_RELEASE))
		return;

	KGI_DEBUG(10, "Key %s, code 0x%.2x, sym %.2x",
		(event->key.type == KII_EV_KEY_PRESS) ? "down" : "up",
		event->key.code, event->key.sym);

	sym = event->key.sym;
	switch (event->key.sym & K_TYPE_MASK) {
	case K_TYPE_SPECIAL:
		if (sym < K_LAST_SPECIAL)
			do_special(f, event);
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

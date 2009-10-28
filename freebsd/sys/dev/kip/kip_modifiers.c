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
 * KII parser.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define	KGI_SYS_NEED_IO
#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#include <sys/kbio.h>
#include <dev/kbd/kbdreg.h>

#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

static void 
do_modifier(kii_focus_t *f, kii_event_t *event)
{
	register kii_unicode_t ksym = event->key.sym;
	kii_u_t effect = f->effect;

	KRN_DEBUG(6, "old modifiers: %.2x %.2x %.2x %.2x, modifier key %.4x",
		f->effect, f->normal, f->locked, f->sticky, ksym);

	/*
	 * The effect of STICKY and LOCKED modifiers is toggled whenever
	 * they are pressed.
	 */
	if ((K_FIRST_STICKY <= ksym) && (ksym < K_LAST_STICKY)) {
		KRN_DEBUG(3, "sticky modifer key");

		if (event->key.type == KII_EV_KEY_PRESS) {
			f->sticky ^= 1 << (ksym - K_FIRST_STICKY);	
		}
	}

	if ((K_FIRST_LOCKED <= ksym) && (ksym < K_LAST_LOCKED)) {
		KRN_DEBUG(3, "locked modifier key");
		if (event->key.type == KII_EV_KEY_PRESS)
			f->locked ^= 1 << (ksym - K_FIRST_LOCKED);
	}

	/*
	 * If a NORMAL modifier is pressed at least once, it goes into
	 * effect and clears the effect of the corresponding LOCKED
	 * modifier. Thus there is no effect in the EFFECTIVE modifiers.
	 * The effect of the NORMAL modifier is not cleared until the
	 * last is released (if pressed multiple times), which clears
	 * the effect of the corresponding LOCKED modifier too.
	 */
	if ((K_FIRST_NORMAL <= ksym) && (ksym < K_LAST_NORMAL)) {
		register kii_u_t mask = 1 << (ksym - K_FIRST_NORMAL);

		KRN_DEBUG(3, "normal modifier key");

		if (event->key.type == KII_EV_KEY_PRESS) {
			f->down_mod[ksym - K_FIRST_NORMAL]++;
			f->normal |=  mask;
			f->locked &= ~mask;
		} else {
			KRN_ASSERT(event->key.type == KII_EV_KEY_RELEASE);
			KRN_ASSERT(f->down_mod[ksym - K_FIRST_NORMAL] > 0);

			if (! --(f->down_mod[ksym - K_FIRST_NORMAL])) {
				f->normal &= ~mask;
				f->locked &= ~mask;
			}
		}
	}

	f->effect = f->normal ^ f->locked ^ f->sticky;

	if ((event->any.type == KII_EV_KEY_RELEASE) && (f->effect != effect) && 
		(f->npadch != K_VOID)) {
		kii_event_t npadch;
		struct timeval tv;

		getmicrotime(&tv);

		npadch.key.type = KII_EV_KEY_PRESS;
		npadch.key.time = tv.tv_sec * 1000000 + tv.tv_usec;
		npadch.key.code = 0xFFFF;
		npadch.key.sym  = f->npadch;
		npadch.key.effect = f->effect;
		npadch.key.normal = f->normal;
		npadch.key.locked = f->locked; 
		npadch.key.sticky = f->sticky;

		if (event->any.dontdispatch == 0)
			kii_put_event(f, &npadch);
	}

	KRN_DEBUG(3, "new modifers: %.2x %.2x %.2x %.2x", f->effect, 
		f->normal, f->locked, f->sticky);
}

static void 
do_ascii(kii_focus_t *f, kii_event_t *event)
{
	kii_s_t base = 10, value = event->key.sym - K_FIRST_ASCII;

	KRN_DEBUG(3, "doing ascii key %.4x", event->key.sym);

	if (value >= 10) {
		value -= 10;
		base = 16;
	}

	f->npadch = (f->npadch == K_VOID) ? value : (f->npadch*base + value);
	f->npadch %= (base == 16) ? 0x10000 : 100000;
}

static kii_unicode_t kii_dead_key[K_LAST_DEAD - K_FIRST_DEAD] = {
	0x0060,	/* ` grave	*/
	0x0027,	/* ' acute	*/
	0x005e,	/* ^ circumflex	*/
	0x007e,	/* ~ tilde	*/
	0x0022,	/* " diaeresis	*/
	0x002c	/* , cedilla	*/
};

static void 
do_dead(kii_focus_t *f, kii_event_t *event)
{
	kii_unicode_t dead = kii_dead_key[event->key.sym - K_FIRST_DEAD];

	KRN_DEBUG(3, "dead key %.4x", dead);
	f->dead = (f->dead == event->key.sym) ? K_VOID : dead;
}

static void 
do_action(kii_focus_t *f, kii_event_t *event)
{
	kii_u_t sym = event->key.sym;

	if ((1 << event->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_RELEASE)) 
		return;

	KRN_DEBUG(6, "key %s, code 0x%.2x, sym %.2x", 
		(event->key.type == KII_EV_KEY_PRESS) ? "down" : "up",
		event->key.code, event->key.sym);

	switch (event->key.sym & K_TYPE_MASK) {
#if defined(CONFIG_KDB)
	case K_TYPE_FUNCTION:
		if ((sym == K_PAUSE) && kdb_on)
			kdb(KDB_REASON_KEYBOARD, 0, f->pt_regs);
		return;
#endif /* CONFIG_KDB */
	case K_TYPE_SHIFT:
		if (sym < K_LAST_SHIFT) 
			do_modifier(f, event);
		return;
	case K_TYPE_ASCII:
		if ((sym < K_LAST_ASCII) 
			&& (event->key.type == KII_EV_KEY_PRESS))
			do_ascii(f, event);
		return;
	case K_TYPE_DEAD:
		if ((sym < K_LAST_DEAD) 
			&& (event->key.type == KII_EV_KEY_PRESS 
			||  event->key.type == KII_EV_KEY_REPEAT)) {
			do_dead(f, event);
		}
		return;
	}
}

void 
kii_handle_input(kii_event_t *event)
{
	kii_focus_t *f;
	kii_u_t mask;
	char *sym_string;

	mask = 1 << event->any.type;	
	sym_string = NULL;
		
	KRN_ASSERT(KII_VALID_FOCUS_ID(event->any.focus));

	f = kiifocus[event->any.focus];
	KRN_ASSERT(f != NULL);

	if (mask & (KII_EM_KEY | KII_EM_POINTER)) {
		event->key.effect = f->effect;
		event->key.normal = f->normal;
		event->key.locked = f->locked;
		event->key.sticky = f->sticky;
	}

	if ((mask & KII_EM_KEY) && (event->key.code < 0x8000)) {
		event->key.sym = keymap_get_keysym(&f->kmap, f->effect,
			event->key.code);

		if (f->flags & KII_FF_CAPS_SHIFT)
			event->key.sym = keymap_toggled_case(event->key.sym);

		sym_string = keysyms_pretty_print(event->key.sym);
		KRN_DEBUG(6, "key %i %s, sym %.4x <%s>", event->key.code,
			(event->key.type == KII_EV_KEY_PRESS) ? "down" : "up",
			event->key.sym, sym_string);
	}

#ifdef	CONFIG_MAGIC_SYSRQ
	if (f->flags & KII_FF_SYSTEM_REQUEST) {
		do_sysrq(f, event);
	}
#endif

	if (mask & (KII_EM_KEY_PRESS | KII_EM_KEY_REPEAT)) {
		kii_event_t composed;

		switch (f->dead) {
		case K_VOID:
			break;
		case K_COMPOSE:
			f->dead = event->key.sym;
			break;
		default:
			switch (K_TYPE(event->key.sym)) {
			case K_TYPE_FUNCTION:	/* Fall thru. */
			case K_TYPE_SPECIAL:	/* Fall thru. */
			case K_TYPE_NUMPAD: 	/* Fall thru. */
			case K_TYPE_CONSOLE:	/* Fall thru. */
			case K_TYPE_CURSOR: 	/* Fall thru. */		
			case K_TYPE_SHIFT: 	/* Fall thru. */
			case K_TYPE_META: 	/* Fall thru. */
				break;
			default:
				composed.key = event->key;
				composed.key.code = 0xFFFF;
				composed.key.sym = keymap_combine_dead(&f->kmap,
					f->dead, event->key.sym);
				KRN_DEBUG(2, "composed %.4x + %.4x -> %.4x",
					f->dead, event->key.sym,
					composed.key.sym);
				f->dead = (K_TYPE(event->key.sym) ==
					K_TYPE_DEAD) ? event->key.sym : K_VOID;
				if (event->any.dontdispatch == 0)
					kii_put_event(f, &composed);
			}
		}
	}

	if (mask & (KII_EM_PTR_RELATIVE | KII_EM_PTR_ABSOLUTE)) {
		if (mask & KII_EM_PTR_RELATIVE) {
			f->ptr.x += event->pmove.x;
			f->ptr.y += event->pmove.y;
		} else {	/* mask & KII_EM_PTR_ABSOLUTE	*/
			f->ptr.x = event->pmove.x;
			f->ptr.y = event->pmove.y;
		}

#define	DO_CLIP(dir)							\
		if (f->ptr.dir < f->ptr_min.dir) {			\
			f->ptr.dir = f->ptr_min.dir;			\
		}							\
		if (f->ptr.dir >= f->ptr_max.dir) {			\
			f->ptr.dir = f->ptr_max.dir - 1;	        \
		}							\
		if (f->focus)	f->focus->ptr.dir = f->ptr.dir;

		DO_CLIP(x)
		DO_CLIP(y)
#undef	DO_CLIP
	}

	/* Local actions. */
	do_action(f, event);

	/* Global actions. */
	kii_action(f, event);

	if (event->any.dontdispatch == 0)
		kii_put_event(f, event);
}

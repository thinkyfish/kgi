/* ---------------------------------------------------------------------------
**      FreeBSD kbd KII driver
** ---------------------------------------------------------------------------
**      Copyright (C)   2003       Nicholas Souchu
**
**      This file is distributed under the terms and conditions of the
**      MIT/X public license. Please see the file COPYRIGHT.MIT included
**      with this software for details of these terms and conditions.
**      Alternatively you may distribute this file under the terms and
**      conditions of the GNU General Public License. Please see the file
**      COPYRIGHT.GPL included with this software for details of these terms
**      and conditions.
** -------------------------------------------------------------------------
**
*/
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define KGI_DBG_LEVEL     1
#endif

#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#include <sys/malloc.h>
#include <sys/kbio.h>
#include <dev/kbd/kbdreg.h>

#include <dev/kgi/kgii.h>
#include <dev/kii/kii.h>

#include <dev/kip/kip.h>

#define MAX_KBD_DRIVERS	4
#define	MAX_NR_KEYS	128

typedef struct kbdriver_softc {
	keyboard_t	*kbd;

	kii_u_t		prev_scancode;
	kii_u8_t	keys[MAX_NR_KEYS];

	kii_input_t	kii_input;
} kbdriver_softc;

static int kbdriver_nr = 0;
static kii_s_t initialized = 0;
static kbdriver_softc kbdriver_data[MAX_KBD_DRIVERS];

/*	scancode to keycode translation
**
**	This is now user-settable. The keycodes 1-88,96-111,119 are fairly
**	standard, and should probably not be changed - changing might confuse X.
**	X also interprets scancode 0x5d (KEY_Begin). For 1-88 keycode equals
**	scancode.
*/

#define E0_KPENTER 96
#define E0_RCTRL   97
#define E0_KPSLASH 98
#define E0_PRSCR   99
#define E0_RALT    100
#define E0_BREAK   101  /* (control-pause) */
#define E0_HOME    102
#define E0_UP      103
#define E0_PGUP    104
#define E0_LEFT    105
#define E0_RIGHT   106
#define E0_END     107
#define E0_DOWN    108
#define E0_PGDN    109
#define E0_INS     110
#define E0_DEL     111

#define E1_PAUSE   119

/*	The keycodes below are randomly located in 89-95,112-118,120-127.
**	They could be thrown away (and all occurrences below replaced by 0),
**	but that would force many users to use the `setkeycodes' utility, where
**	they needed not before. It does not matter that there are duplicates, as
**	long as no duplication occurs for any single keyboard.
*/

#define SC_LIM 89

#define FOCUS_PF1 85           /* actual code! */
#define FOCUS_PF2 89
#define FOCUS_PF3 90
#define FOCUS_PF4 91
#define FOCUS_PF5 92
#define FOCUS_PF6 93
#define FOCUS_PF7 94
#define FOCUS_PF8 95
#define FOCUS_PF9 120
#define FOCUS_PF10 121
#define FOCUS_PF11 122
#define FOCUS_PF12 123

#define JAP_86     124

/*	tfj@olivia.ping.dk: The four keys are located over the numeric 
**	keypad, and are labelled A1-A4. It's an rc930 keyboard, from
**	Regnecentralen/RC International, Now ICL. Scancodes: 59, 5a, 5b, 5c.
*/

#define RGN1 124
#define RGN2 125
#define RGN3 126
#define RGN4 127


/* BTC */
#define E0_MACRO   112
/* LK450 */
#define E0_F13     113
#define E0_F14     114
#define E0_HELP    115
#define E0_DO      116
#define E0_F17     117
#define E0_KPMINPLUS 118

/*	kkoller@nyx10.cs.du.edu: My OmniKey generates e0 4c for the "OMNI" 
**	key and the right alt key does nada.
*/

#define E0_OK	124

/*	New microsoft keyboard is rumoured to have
**	e0 5b (left window button),	LBANNER or Windows_L
**	e0 5c (right window button),	RBANNER or Windows_R
**	e0 5d (menu button).		RMENU   or TaskMan
*/

#define E0_MSLW	125
#define E0_MSRW	126
#define E0_MSTM	127


static kii_u8_t high_keys[128 - SC_LIM] =
{
	RGN1, RGN2, RGN3, RGN4, 0, 0, 0,                   /* 0x59-0x5f */
	0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x60-0x67 */
	0, 0, 0, 0, 0, FOCUS_PF11, 0, FOCUS_PF12,          /* 0x68-0x6f */
	0, 0, 0, FOCUS_PF2, FOCUS_PF9, 0, 0, FOCUS_PF3,    /* 0x70-0x77 */
	FOCUS_PF4, FOCUS_PF5, FOCUS_PF6, FOCUS_PF7,        /* 0x78-0x7b */
	FOCUS_PF8, JAP_86, FOCUS_PF10, 0                   /* 0x7c-0x7f */
};

static kii_u8_t e0_keys[128] =
{
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x00-0x07 */
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x08-0x0f */
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x10-0x17 */
	0, 0, 0, 0, E0_KPENTER, E0_RCTRL, 0, 0,		/* 0x18-0x1f */
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x20-0x27 */
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x28-0x2f */
	0, 0, 0, 0, 0, E0_KPSLASH, 0, E0_PRSCR,		/* 0x30-0x37 */
	E0_RALT, 0, 0, 0, 0, E0_F13, E0_F14, E0_HELP,	/* 0x38-0x3f */
	E0_DO, E0_F17, 0, 0, 0, 0, E0_BREAK, E0_HOME,	/* 0x40-0x47 */
	E0_UP, E0_PGUP, 0, E0_LEFT, E0_OK, E0_RIGHT, E0_KPMINPLUS, E0_END,/* 0x48-0x4f */
	E0_DOWN, E0_PGDN, E0_INS, E0_DEL, 0, 0, 0, 0,	/* 0x50-0x57 */
	0, 0, 0, E0_MSLW, E0_MSRW, E0_MSTM, 0, 0,	/* 0x58-0x5f */
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x60-0x67 */
	0, 0, 0, 0, 0, 0, 0, E0_MACRO,			/* 0x68-0x6f */
	0, 0, 0, 0, 0, 0, 0, 0,				/* 0x70-0x77 */
	0, 0, 0, 0, 0, 0, 0, 0				/* 0x78-0x7f */
};


/* The parser takes scancodes from the RAW output of a FreeBSD kbd
 * driver.
 */
static
void kbdriver_parser(kii_input_t *input, kii_event_t *event, int scancode)
{
	kbdriver_softc *sc = input->priv.priv_ptr;
	kii_u8_t release;
	kii_u_t keycode;

	event->any.focus = input->focus;
	event->any.device = input->id;
	event->any.time = 0;			/* XXX no jiffies */

	/* Perform raw treatment at end to enable switches
	 * before delivery of key.
	 */
	if (input->report & KII_EM_RAW_DATA) {

		event->raw.type	  = KII_EV_RAW_DATA;
		event->raw.size	  = sizeof(kii_any_event_t) + 4;
		*(int *)&event->raw.data = scancode;

		kii_handle_input(event);
	}

	scancode &= 0xFF;

	if (scancode == 0xE0 || scancode == 0xE1) {

		sc->prev_scancode = scancode;
		goto end;
	}

	release = scancode & 0x80;
	scancode &= 0x7F;

	if (sc->prev_scancode) {

		/*	usually it will be 0xe0, but a Pause key generates
		**	e1 1d 45 e1 9d c5 when pressed, and nothing when
		**	released.
		*/
		if (sc->prev_scancode != 0xe0) {

			if (sc->prev_scancode == 0xe1 && scancode == 0x1d) {

				sc->prev_scancode = 0x100;
				goto end;
	
			} else {

				if (sc->prev_scancode == 0x100 &&
					scancode == 0x45) {
	
					keycode = E1_PAUSE;
					sc->prev_scancode = 0;

				} else {

					KRN_ERROR("unknown E1 sequence");
					sc->prev_scancode = 0;
					goto end;
				}
			}

		} else {	/* if (sc->prev_scancode != 0xe0) { ... */

			sc->prev_scancode = 0;

			/*	The keyboard maintains its own internal capsi
			**	lock and num lock statuses. In caps lock mode
			**	E0 AA precedes make code and E0 2A follows
			**	break code. In num lock mode, E0 2A precedes
			**	make code and E0 AA follows break code. We do
			**	our own book-keeping, so we just ignore these.
			**
			**	For my keyboard there is no caps lock mode,
			**	but there are both Shift-L and Shift-R modes.
			**	The former mode generates E0 2A / E0 AA pairs,
			**	the latter E0 B6 / E0 36 pairs. So, we should
			**	also ignore the latter. -- aeb@cwi.nl
			*/
			if (scancode == 0x2a || scancode == 0x36) {

				goto end;
			}

			if (e0_keys[scancode]) {

				keycode = e0_keys[scancode];

			} else {

				KRN_ERROR("unknown scancode e0 %02x", scancode);
				goto end;
			}
		}

	} else {  /* if (sc->prev_scancode) ... */

		if (scancode >= SC_LIM) {

		        /*	This happens with the FOCUS 9000 keyboard.
			**	Its keys PF1..PF12 are reported to generate
			**	55 73 77 78 79 7a 7b 7c 74 7e 6d 6f. Moreover,
			**	unless repeated, they do not generate key-down
			**	events, so we have to zero <release> below.
			**
			**	Also, Japanese 86/106 keyboards are reported
			**	to generate 0x73 and 0x7d for \ - and \ |
			**	respectively.  
			**
			**	Also, some Brazilian keyboard is reported to
			**	produce 0x73 and 0x7e for \ ? and KP-dot,
			**	respectively.
			*/

			keycode = high_keys[scancode - SC_LIM];

			if (! keycode) {

				KRN_ERROR("unknown scancode %2x", scancode);
				goto end;
			}

		} else {

			 keycode = scancode;
		}
	}

	event->key.size = sizeof(kii_key_event_t);
	event->key.code = keycode;

	if (release) {

		event->key.type = KII_EV_KEY_RELEASE;

		/* XXX test_and_clear_bit */
		if (sc->keys[keycode]) {
			sc->keys[keycode] = 0;

			/*	unexpected, but this can happen: maybe this
			**	was a key release for a FOCUS 9000 PF key;
			*/
			if (keycode >= SC_LIM || keycode == 85) {
				
				event->key.type = KII_EV_KEY_PRESS;
				if (input->report & KII_EV_KEY_PRESS) {
					
					kii_handle_input(event);
				}
				event->key.type = KII_EV_KEY_RELEASE;
			}
		}
	} else {
		/* XXX test_and_set_bit */
		event->key.type = sc->keys[keycode] ?
				KII_EV_KEY_REPEAT : KII_EV_KEY_PRESS;
		sc->keys[keycode] = 1;
	}

	if (input->report & (1 << event->key.type)) {

		kii_handle_input(event);
	}

 end:	
	return;
}

static
int kbdriver_event(keyboard_t *kbd, int evt, void *arg)
{
	kbdriver_softc *sc;
	kii_event_t event;
	int c;

	sc = (kbdriver_softc *)arg;

	switch (evt) {
	case KBDIO_KEYINPUT:
		break;
	case KBDIO_UNLOADING:			/* XXX */
	default:
		return EINVAL;
	}

	while ((*kbdsw[kbd->kb_index]->check_char)(kbd)) {
		c = (*kbdsw[kbd->kb_index]->read_char)(kbd, FALSE);

		if (c == NOKEY)
			break;

		if (c == ERRKEY)	/* XXX: ring bell? */
			continue;

		bzero(&event, sizeof(event));
		kbdriver_parser(&sc->kii_input, &event, c);
	}

	return 0;
}

/* Get a scancode if any */
static int
kbdriver_poll(kii_input_t *input)
{
	kbdriver_softc *sc = input->priv.priv_ptr;
	keyboard_t *kbd = sc->kbd;
	int c;

	c = (*kbdsw[(kbd)->kb_index]->read)((kbd), 0);
	return c;
}

void
kip_kbd_register(keyboard_t *kbd, int index) 
{
	kbdriver_softc *sc;
	kii_error_t kii_error;
	int kbd_mode, i;

	sc = &kbdriver_data[index];

	if (!sc->kbd) {

		sc->kbd = kbd;

		/*
		** Try to get exclusive access to this kbd
		*/
		KRN_DEBUG(1, "trying to allocate keyboard '%s'",
			  kbd->kb_name);

		i = kbd_allocate(kbd->kb_name, kbd->kb_unit, sc,
				 kbdriver_event, (void *)sc);

		/* Check if allocation succeeded */
		if (i >= 0) {
			/*
			** Put the keyboard in raw mode so that we'll
			** receive directly the scancodes from lower
			** layers.
			*/
			kbd_mode = K_RAW;
			(*kbdsw[kbd->kb_index]->ioctl)(kbd, KDSKBMODE,
						       (caddr_t)&kbd_mode);

			snprintf(sc->kii_input.vendor, KII_MAX_VENDOR_STRING,
				 "KII FreeBSD keyboard");
			snprintf(sc->kii_input.model, KII_MAX_VENDOR_STRING,
				 kbd->kb_name);

			sc->kii_input.events = KII_EM_KEY | KII_EM_RAW_DATA;
			sc->kii_input.report = KII_EM_KEY | KII_EM_RAW_DATA;
			sc->kii_input.priv.priv_ptr = sc;
			sc->kii_input.Poll = kbdriver_poll;
			sc->kii_input.Parse = kbdriver_parser;

			/* Register to the focus with same id as the kbd */
			kii_error = kii_register_input(kbdriver_nr, &sc->kii_input);

			KRN_NOTICE("%s: keyboard %d registered on focus %d with error %d",
				   __FUNCTION__, index, kbdriver_nr, kii_error);

			kbdriver_nr ++;

		} else {
			/* Free the kbdriver_data entry */
			sc->kbd = NULL;
		}
		
	}
}

static void
kbdriver_init(void)
{

	if (initialized)
		return;

	initialized = 1;

	/* RAZ the allocation area especially to have kbd field NULL */
	bzero(kbdriver_data, sizeof(kbdriver_data));
	kbdriver_nr = 0;

	return;
}

static int
kbdriver_modevent(module_t mod, int type, void *unused)
{
	kbdriver_softc *sc = NULL;
	int s;

	switch (type) {

	case MOD_LOAD:
		/* Grab any kbd already registered */
		kbdriver_init();
		break;

	case MOD_UNLOAD:
		s = spltty();

		while (kbdriver_nr>0) {

			sc = &kbdriver_data[kbdriver_nr];

			if (sc->kbd && KBD_IS_VALID(sc->kbd)) {
				kii_unregister_input(&sc->kii_input);

				kbd_release(sc->kbd, (void *)sc);

				KRN_NOTICE("%s: keyboard %d unregistered",
					__FUNCTION__, kbdriver_nr);
			}
			sc->kbd = NULL;
			kbdriver_nr --;
		}

		splx(s);
		break;

	default:
		break;
	}

	return 0;
};

static moduledata_t kbdriver_mod = {
	"kbdriver",
	kbdriver_modevent,
	0
};

DECLARE_MODULE(kbdriver, kbdriver_mod, SI_SUB_DRIVERS, SI_ORDER_FIRST);
MODULE_VERSION(kbdriver, 1);

/* ---------------------------------------------------------------------------
**	PS/2 auxiliary keyboard device driver
** ---------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** -------------------------------------------------------------------------
** 
**	$Log: kbd-i386.c,v $
**	Revision 1.1.1.1  2000/04/18 08:50:51  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#define DRIVER_NAME	"PS/2 kbd/aux driver"
#define DRIVER_REV	"$Revision: 1.1.1.1 $"
#define	DEBUG_LEVEL	1

#define	KGI_SYS_NEED_IO

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/random.h>
#include <linux/kgii.h>

#include <kgi/debug.h>

#if defined(__alpha__) && !defined(CONFIG_PCI)
#	define	AUX_PTR_IRQ	9	/* Jensen is odd indeed */
#	define	AUX_KBD_IRQ	1
#else
#	define	AUX_PTR_IRQ	12
#	define	AUX_KBD_IRQ	1
#endif

unsigned int aux_ptr_irq = AUX_PTR_IRQ;
unsigned int aux_kbd_irq = AUX_KBD_IRQ;
int aux_ptr_focus = -1;
int aux_kbd_focus = -1;

#define	AUX_TIMEOUT_CONST	0x40000

/*
**	controller ports
*/
#define	AUX_IN			0x60	/* input (from dev) buffer	*/
#define	AUX_OUT			0x60	/* output (to dev) buffer	*/
#define	AUX_CMD			0x64	/* command buffer		*/
#define	AUX_STATUS		0x64	/* status reg			*/

/*
**	controller status bits
*/
#define AUX_STATUS_IBF		0x01	/* inbuffer (from device) full	*/
#define AUX_STATUS_OBF		0x02	/* outbuffer (to device) full	*/
#define	AUX_STATUS_OBT		0x20	/* output (to device) timeout	*/
#define	AUX_STATUS_IBT		0x40	/* input (from device) timeout	*/
#define	AUX_STATUS_PERR		0x80	/* transmission error		*/

/*
**	mode register
*/
#define	AUX_MODE_KBD_INTS_ON	0x01	/* enable keyboard interrupts	*/
#define	AUX_MODE_PTR_INTS_ON	0x02	/* enable pointer interrupts	*/
#define	AUX_MODE_SYSFLAG	0x04	/* system flag of status reg	*/
#define	AUX_MODE_IGNORE_SWITCH	0x08	/* ignore keyboard switch	*/
#define	AUX_MODE_KBD_OFF	0x10	/* keyboard interface off	*/
#define	AUX_MODE_PTR_OFF	0x20	/* pointer interface off	*/
#define	AUX_MODE_XT_SCANCODES	0x40	/* translate AT or XT scancodes */

/*
**	psaux controller commands
*/
#define	AUX_CMD_READ_MODE	0x20	/* read command register	*/
#define	AUX_CMD_WRITE_MODE	0x60	/* write command register	*/
#define	AUX_CMD_SELF_TEST	0xAA	/* controller self test		*/

#define	AUX_CMD_KBD_DISABLE	0xAD	/* switch keyboard interf. off	*/
#define	AUX_CMD_KBD_ENABLE	0xAE	/* switch keyboard interf. on	*/
#define	AUX_CMD_KBD_TEST	0xAB	/* test keyboard interface	*/

#define	AUX_CMD_PTR_DISABLE	0xA7	/* switch auxiliary off		*/
#define	AUX_CMD_PTR_ENABLE	0xA8	/* switch auxiliary on		*/
#define	AUX_CMD_PTR_TEST	0xA9	/* test auxiliary device	*/
#define	AUX_CMD_PTR_INTS_ON	0x47	/* enable auxiliary interrupts	*/
#define	AUX_CMD_PTR_INTS_OFF	0x65	/* disable auxiliary interrupts	*/
#define	AUX_CMD_PTR_WRITE	0xD4	/* send data to aux. device	*/

#define AUX_CMD_AMI_IDSTRING    0xA0    /* Get Copyright AMIBIOS string */
#define AUX_CMD_AMI_REV         0xA1    /* Get AMIBIOS kbd revision #   */

/* Controller reply */
#define AUX_TEST_SUCCESS        0x55    /* Controller self-test success    */
#define AUX_IF_TEST_SUCCESS     0x00    /* kbd interface test success */
#define AUX_IF_TEST_CLOCK_LO    0x01    /* kbd clock stuck lo */
#define AUX_IF_TEST_CLOCK_HI    0x02    /* kbd clock stuck hi */
#define AUX_IF_TEST_DATA_LO     0x03    /* kbd data stuck lo  */
#define AUX_IF_TEST_DATA_HI     0x04    /* kbd data stuck hi  */


/*
**	common device commands
*/
#define	DEV_CMD_RESET		0xFF	/* reset device			*/
#define	DEV_CMD_ENABLE		0xF4	/* enable device		*/
#define	DEV_CMD_DISABLE		0xF5	/* disable device		*/

/*
**	common device replies
*/
#define	DEV_ACKNOWLEDGE		0xFA	/* acknowledge			*/
#define	DEV_RESEND		0xFE	/* resend last byte		*/
#define DEV_BAT_SUCCESS         0xAA    /* Basic Assurance Test success */
#define DEV_TEST_ERROR          0xFC    /* Self test error */

/*
**	pointer device commands
*/
#define	PTR_CMD_SET_RES		0xe8	/* set resolution		*/
#define	PTR_CMD_SET_SCALE11	0xe6	/* set 1:1 scaling		*/
#define	PTR_CMD_SET_SCALE21	0xe7	/* set 2:1 scaling		*/
#define PTR_CMD_GET_SCALE	0xe9	/* get scaling factor		*/
#define PTR_CMD_SET_STREAM	0xea	/* set stream mode		*/
#define PTR_CMD_SET_SAMPLE	0xf3	/* set sample rate		*/

/*
**	keyboard device commands
*/
#define	KBD_CMD_SET_LED		0xED	/* set mode indicators		*/
#	define	KBD_LED_SCROLL	0x01	/* scroll-lock			*/
#	define	KBD_LED_NUM	0x02	/* num-lock			*/
#	define	KBD_LED_CAPS	0x04	/* caps-lock			*/
#define	KBD_CMD_ECHO		0xEE	/* ping to keyboard		*/
#define	KBD_CMD_SCANCODE	0xF0	/* get/set scancode set		*/
#	define	KBD_SC_GET	0x00	/* get current scancode set	*/
#	define	KBD_SC_1	0x01	/* set scancode set 1 (PC/XT)	*/
#	define	KBD_SC_2	0x02	/* set scancode set 2		*/
#	define	KBD_SC_3	0x03	/* set scancode set 3		*/
#define	KBD_CMD_ID		0xF2	/* read device ID		*/
#define	KBD_CMD_TYPEMATICRATE	0xF3	/* set typematic rate		*/
#define	KBD_CMD_RESEND		0xFE	/* resend last byte		*/

/*
**	keyboard device replies
*/
#define	KBD_OVERRUN1		0x00	/* scancode buf overrun (set1)	*/
#define	KBD_TEST_COMPLETE	0xAA	/* self test passed		*/
#define	KBD_ECHO		0xEE	/* diagnostic echo		*/
#define	KBD_BREAK_CODE		0xF0	/* break code prefix		*/
#define	KBD_RESEND		0xFE	/* transmission error, resend	*/
#define	KBD_OVERRUN2		0xFF	/* scancode buf overrun (set2)	*/
#define	KBD_OVERRUN3		0xFF	/* scancode buf overrun (set3)	*/

/*
**	misc I/O and helper functions
*/

static void aux_out(kii_u8_t val)
{
	kii_u_t i = AUX_TIMEOUT_CONST;

	while ((io_in8(AUX_STATUS) & AUX_STATUS_OBF) && --i) {
	}

	if (i) {

		KRN_DEBUG(2, "aux_out <- %.2x", val);
		io_out8(val, AUX_OUT);

	} else {

		KRN_DEBUG(2, "aux_out timed out");
	}
}

static kii_u8_t aux_in(void)
{
	kii_u_t i = AUX_TIMEOUT_CONST;

	while ((~io_in8(AUX_STATUS) & AUX_STATUS_IBF) && --i) {
	}

	if (i) {

		kii_u8_t foo = io_in8(AUX_IN);
		KRN_DEBUG(2, "aux_in -> %.2x, i == %i", foo, i);
		return foo;

	} else {

		KRN_DEBUG(2, "aux_in timed out");
		return 0xFF;
	}
}

static inline void aux_cmd(kii_u8_t cmd)
{
	kii_u_t i = AUX_TIMEOUT_CONST;

	while ((io_in8(AUX_STATUS) & AUX_STATUS_OBF) && --i) {
	}

	if (i) {

		KRN_DEBUG(2, "aux_cmd <- %.2x", cmd);
		io_out8(cmd, AUX_CMD);

	} else {

		KRN_DEBUG(2, "aux_cmd timed out");
	}
}

static inline void aux_flush(void)
{
	kii_u_t i;

	i = AUX_TIMEOUT_CONST;
	do {

		while ((~io_in8(AUX_STATUS) & AUX_STATUS_IBF) && --i) {
		}

		if (i) {

			i = AUX_TIMEOUT_CONST;
			io_in8(AUX_IN);
		} 

	} while (i);
}

/*	Send a datum to the device connected to the kbd port.
*/
static inline kii_u8_t aux_kbd_send(kii_u8_t dat)
{
	kii_u_t i = 3;
	kii_u8_t reply;

	do {
		aux_out(dat);
		reply = aux_in();

	} while ((reply == DEV_RESEND) && i--);

	return reply;
}

/*	send a datum to the device connected to the ptr port.
*/
static inline kii_u8_t aux_ptr_send(kii_u8_t dat)
{
	kii_u8_t reply;
	kii_u_t i = 3;

	do {
		aux_cmd(AUX_CMD_PTR_WRITE);
		aux_out(dat);
		reply = aux_in();

	} while ((reply == DEV_RESEND) && i--);

	return reply;
}

/*
**	the interrupt handlers
*/

static void *aux_kbd_parser_priv = NULL;
static void *aux_ptr_parser_priv = NULL;
static void (*aux_kbd_parser)(void *priv, kii_u8_t data, void *pt_regs) = NULL;
static void (*aux_ptr_parser)(void *priv, kii_u8_t data, void *pt_regs) = NULL;

static void aux_kbd_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	kii_u8_t status, data;
	aux_cmd(AUX_CMD_KBD_DISABLE);

	status = io_in8(AUX_STATUS);
	if ((status & AUX_STATUS_OBT) == 0) {

		data = io_in8(AUX_IN);	

		if (aux_kbd_parser) {

			aux_kbd_parser(aux_kbd_parser_priv, data, regs);
		}
	}

	aux_cmd(AUX_CMD_KBD_ENABLE);
}

static void aux_ptr_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	kii_u8_t data;

	aux_cmd(AUX_CMD_PTR_DISABLE);

	if (io_in8(AUX_STATUS) & AUX_STATUS_OBT) {

		data = io_in8(AUX_IN);

		if (aux_ptr_parser) {
	
			aux_ptr_parser(aux_ptr_parser_priv, data, regs);
		}
	}

	aux_cmd(AUX_CMD_PTR_ENABLE);
}

static io_region_t aux_io_region =
{
	PCICFG_NULL,	/* device		*/
	IO_NULL, 	/* base_virt		*/
	0x60,0,0,	/* base_{io, bus, phys}	*/
	16,		/* size			*/
	0xFFFF,		/* decode		*/
	"kbd/aux interface"
};

static int aux_ptr = 0, aux_kbd = 0;

/*
**	keyboard  parser
*/

#define	BYTES(x)	(((x) + 7) & ~7)
#define	MAX_NR_KEYS	128

struct aux_parser_kbd_data
{
	kii_input_t	input;

	kii_u_t		prev_scancode;
	kii_u8_t	keys[BYTES(MAX_NR_KEYS)];	
};

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

#if 0
static int kbd_set_keycode(kii_u_t scancode, kii_u_t keycode)
{
	if ((scancode < SC_LIM) || (scancode > 255) || (keycode > 127)) {

		return -EINVAL;
	}

	if (scancode < 128) {

		high_keys[scancode - SC_LIM] = keycode;

	} else {

		e0_keys[scancode - 128] = keycode;
	}

	return EOK;
}

static kii_u_t kbd_get_keycode(kii_u_t scancode)
{
	return ((scancode < SC_LIM) || (scancode > 255))
			? -EINVAL
			: (scancode < 128)
				? high_keys[scancode - SC_LIM]
				: e0_keys[scancode - 128];
}
#endif

static void aux_parser_keyboard(void *priv, kii_u8_t scancode, void *regs)
{
	struct aux_parser_kbd_data *kbd = (struct aux_parser_kbd_data *) priv;
	kii_event_t event;
	kii_u8_t release;
	kii_u_t keycode;

	add_keyboard_randomness(scancode);

	switch (scancode) {

	case 0:
		KRN_ERROR("keyboard buffer overflow");
		return;

	case DEV_ACKNOWLEDGE:
	case DEV_RESEND:
		return;

	case 0xFF:
		KRN_ERROR("keyboard error");
		return;
	}

	event.any.focus  = kbd->input.focus;
	event.any.device = kbd->input.id;
	event.any.time   = jiffies;

	if (kbd->input.report & KII_EM_RAW_DATA) {

		event.raw.type	  = KII_EV_RAW_DATA;
		event.raw.size	  = sizeof(kii_any_event_t) + 1;
		event.raw.data[0] = scancode;

		kii_handle_input(&event, regs);
	}

	if (scancode == 0xE0 || scancode == 0xE1) {

		kbd->prev_scancode = scancode;
		return;
	}

	release = scancode & 0x80;
	scancode &= 0x7F;

	if (kbd->prev_scancode) {

		/*	usually it will be 0xe0, but a Pause key generates
		**	e1 1d 45 e1 9d c5 when pressed, and nothing when
		**	released.
		*/
		if (kbd->prev_scancode != 0xe0) {

			if (kbd->prev_scancode == 0xe1 && scancode == 0x1d) {

				kbd->prev_scancode = 0x100;
				return;
	
			} else {

				if (kbd->prev_scancode == 0x100 &&
					scancode == 0x45) {
	
					keycode = E1_PAUSE;
					kbd->prev_scancode = 0;

				} else {

					KRN_ERROR("unknown E1 sequence");
					kbd->prev_scancode = 0;
					return;
				}
			}

		} else {	/* if (kbd->prev_scancode != 0xe0) { ... */

			kbd->prev_scancode = 0;

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

				return;
			}

			if (e0_keys[scancode]) {

				keycode = e0_keys[scancode];

			} else {

				KRN_ERROR("unknown scancode e0 %02x", scancode);
				return;
			}
		}

	} else {  /* if (kbd->prev_scancode) ... */

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
				return;
			}

		} else {

			 keycode = scancode;
		}
	}

	event.key.size = sizeof(kii_key_event_t);
	event.key.code = keycode;

	if (release) {

		event.key.type = KII_EV_KEY_RELEASE;
		if (! test_and_clear_bit(keycode, kbd->keys)) {

			/*	unexpected, but this can happen: maybe this
			**	was a key release for a FOCUS 9000 PF key;
			*/
			if (keycode >= SC_LIM || keycode == 85) {

				event.key.type = KII_EV_KEY_PRESS;
				if (kbd->input.report & KII_EV_KEY_PRESS) {

					kii_handle_input(&event, regs);
				}
				event.key.type = KII_EV_KEY_RELEASE;
			}
		}

	} else {

		event.key.type = test_and_set_bit(keycode, kbd->keys)
			? KII_EV_KEY_REPEAT : KII_EV_KEY_PRESS;
	}

	if (kbd->input.report & (1 << event.key.type)) {

		kii_handle_input(&event, regs);
	}
}

/*
**	pointer parser
*/

struct aux_parser_ptr_data
{
	kii_input_t	input;

	kii_u_t		state;
	kii_u_t		b, last_b;
	kii_s_t		dx, dy;
};

static void aux_parser_pointer(void *priv, kii_u8_t data, void *regs)
{
	struct aux_parser_ptr_data *ptr = (struct aux_parser_ptr_data *) priv;
	kii_event_t event;

	add_mouse_randomness(data);

	switch (ptr->state) {

	case 0:
		ptr->b = data;
		ptr->state++;
		return;

	case 1:
		ptr->dx = data;
		if (ptr->b & 0x10) {

			ptr->dx |= (-1 & ~0xFF);
		}
		ptr->state++;
		return;

	case 2:
		ptr->dy = data;
		if (ptr->b & 0x20) {

			ptr->dy |= (-1 & ~0xFF);
		}
		ptr->state = 0;
		ptr->b &= 7;
	}


	KRN_DEBUG(2, "packet %.2x %i, %i", ptr->b, ptr->dx, ptr->dy);
	/* !!!	do mouse acceleration here !!! */

	event.any.focus  = ptr->input.focus;
	event.any.device = ptr->input.id;
	event.any.time   = jiffies;

	if (ptr->dx || ptr->dy) {

		event.pmove.size = sizeof(kii_pmove_event_t);

		event.pmove.wheel = 0;
		event.pmove.dummy[0] = event.pmove.dummy[1] = 0;

		event.pmove.x =  ptr->dx;
		event.pmove.y = -ptr->dy;

		if (ptr->input.report & KII_EM_PTR_RELATIVE) {

			KRN_DEBUG(2, "KII_EV_PTR_RELATIVE");
			event.pmove.type = KII_EV_PTR_RELATIVE;
			kii_handle_input(&event, regs);
		}
	}

	/*	Change in button state?
	*/
	if ((event.pbutton.button = ptr->last_b ^ ptr->b)) {

		event.pbutton.size = sizeof(kii_pbutton_event_t);
		ptr->last_b = event.pbutton.state = ptr->b;
		event.pbutton.type =
			(event.pbutton.button & event.pbutton.state)
			? KII_EV_PTR_BUTTON_PRESS : KII_EV_PTR_BUTTON_RELEASE;

		if (ptr->input.report & (1 << event.pbutton.type)) {

			KRN_DEBUG(2, "%s", 
			       (event.pbutton.type == KII_EV_PTR_BUTTON_PRESS)
			       ? "KII_EV_PTR_BUTTON_PRESS" 
			       : "KII_EV_PTR_BUTTON_RELEASE");
			kii_handle_input(&event, regs);
		}
	}	
}

typedef union
{
	kii_input_t			input;
	struct aux_parser_kbd_data	kbd;
	struct aux_parser_ptr_data	ptr;

} aux_private;

static aux_private aux_ptr_priv, aux_kbd_priv;

#define	SIG_KEYBOARD \
		0x83abfa:	/* Standard XT keyboard response	*/\
	case	0xfffffa:	/* Old AT keyboards just send ACK	*/\
	case	0x41abfa	/* MF2 in translate mode		*/

#define	SIG_POINTER \
		0xff00fa	/* standard PS/2 mouse response		*/

static int aux_init_input(aux_private *priv, kii_u_t sig, char *port,
	kii_s_t focus)
{
	memset(priv, 0, sizeof(*priv));
	priv->input.focus = -1;
	priv->input.id	= -1;
	strcpy (priv->input.vendor, "IBM");

	switch (sig) {

	case SIG_KEYBOARD:
		priv->input.events = KII_EM_KEY | KII_EM_RAW_DATA;
		priv->input.report = KII_EM_KEY | KII_EM_RAW_DATA;
		sprintf(priv->input.model, "PS/2 keyboard (%s)", port);
		break;

	case SIG_POINTER:
		priv->input.events = KII_EM_POINTER & ~KII_EM_PTR_ABSOLUTE;
		priv->input.report = KII_EM_PTR_RELATIVE | KII_EM_PTR_BUTTON;
		sprintf(priv->input.model, "auxiliary pointer (%s)", port);
		break;

	default:
		return -EINVAL;
	}

	KRN_DEBUG(0, "registering %s %s to focus %i",
		priv->input.vendor, priv->input.model, focus);
	return kii_register_input(focus, &(priv->input));
}


#ifndef	MODULE
static int aux_init_devices(void)
#else
int init_module(void)
#endif
{
	kii_u8_t  res;
	int	signature = 0;


	/*	claim region, disable interrupts and flush any pending input.
	*/
	if (io_check_region(&aux_io_region)) {

		KRN_DEBUG(0, "aux_io_region busy");
		return -EBUSY;
	}
	io_claim_region(&aux_io_region);

	aux_cmd(AUX_CMD_WRITE_MODE);
	aux_out(AUX_MODE_XT_SCANCODES);
	aux_flush();

	/*	do controller self test.
	*/
	aux_cmd(AUX_CMD_SELF_TEST);
	res = aux_in();
	if (res == DEV_TEST_ERROR) {

		KRN_ERROR("controller self test failed");
		io_free_region(&aux_io_region);
		return -ENODEV;
	}
	if (res != AUX_TEST_SUCCESS) {

		KRN_ERROR("Unknown controller response (%x) to self-test", res);
		io_free_region(&aux_io_region);
		return -ENODEV;
	}

	/*	test kbd and ptr interface and claim interrupts
	*/
	aux_cmd(AUX_CMD_KBD_TEST);
	res = aux_in();
	aux_kbd = 0;
	switch (res) {
	case AUX_IF_TEST_SUCCESS:
		aux_kbd = !request_irq(aux_kbd_irq, aux_kbd_interrupt, 0, 
			"aux kbd", &aux_kbd_priv);
		if(!aux_kbd) {

			KRN_ERROR("Failed to get ptr IRQ (%n)", aux_kbd_irq);
		}
		break;
	case AUX_IF_TEST_CLOCK_LO:
		KRN_DEBUG(1,"kbd interface clock line is stuck low");
		break;
	case AUX_IF_TEST_CLOCK_HI:
		KRN_DEBUG(1,"kbd interface clock line is stuck high");
		break;
	case AUX_IF_TEST_DATA_LO:
		KRN_DEBUG(1,"kbd interface data line is stuck low");
		break;
	case AUX_IF_TEST_DATA_HI:
		KRN_DEBUG(1,"kbd interface clock line is stuck high");
		break;
	default:
		KRN_DEBUG(1,"Unknown response (%x) to kbd interface test", res);
	}

	aux_cmd(AUX_CMD_PTR_TEST);
	res = aux_in();
	aux_ptr = 0;
	switch(res) {
	case AUX_IF_TEST_SUCCESS:
		aux_ptr = !request_irq(aux_ptr_irq, aux_ptr_interrupt, 0, 
			"aux ptr", &aux_ptr_priv);
		if(!aux_ptr) {
			KRN_ERROR("Failed to get ptr IRQ (%n)", aux_ptr_irq);
		}
		break;
	case AUX_IF_TEST_CLOCK_LO:
		KRN_DEBUG(1,"ptr interface clock line is stuck low");
		break;
	case AUX_IF_TEST_CLOCK_HI:
		KRN_DEBUG(1,"ptr interface clock line is stuck high");
		break;
	case AUX_IF_TEST_DATA_LO:
		KRN_DEBUG(1,"ptr interface data line is stuck low");
		break;
	case AUX_IF_TEST_DATA_HI:
		KRN_DEBUG(1,"ptr interface clock line is stuck high");
		break;
	default:
		KRN_DEBUG(1,"Unknown response (%x) to ptr interface test", res);
	}

	aux_cmd(AUX_CMD_PTR_DISABLE);
	aux_cmd(AUX_CMD_KBD_DISABLE);

	if (! (aux_ptr || aux_kbd)) {

		KRN_ERROR("no devices detected or resources busy");
		io_free_region(&aux_io_region);
		return -ENODEV;
	}

	if (aux_ptr) {

		aux_cmd(AUX_CMD_PTR_ENABLE);

		aux_ptr_send(DEV_CMD_RESET);
		res = aux_in();
		if (res != DEV_BAT_SUCCESS) {
	
			KRN_DEBUG(1,"Unknown BAT response (%x) from ptr port",
				res);
		}
		signature =  aux_ptr_send(KBD_CMD_ID);
		signature |= aux_in() << 8;
		signature |= aux_in() << 16;

		switch (signature) {

		case SIG_POINTER:
			KRN_DEBUG(0, "pointer detected on aux ptr port");
			aux_ptr_send(DEV_CMD_ENABLE);
			aux_ptr_parser = aux_parser_pointer;
			if (aux_ptr_focus < 0) {

				aux_ptr_focus = 0;
			}
			break;

		case SIG_KEYBOARD:
			KRN_DEBUG(0, "keyboard detected on aux ptr port");
			aux_ptr_send(DEV_CMD_ENABLE);
			if (aux_ptr_focus < 0) {

				aux_ptr_focus = 1;
			}
			aux_ptr_parser = aux_parser_keyboard;
			break;

		default:
			KRN_DEBUG(0, "unknown signature %.6x from ptr",
				signature);
			free_irq(aux_ptr_irq, NULL);
			aux_ptr = 0;
		}

		if (aux_ptr && aux_init_input(&aux_ptr_priv, signature,
			"ptr", aux_ptr_focus)) {

			KRN_DEBUG(0, "%s %s registration failed",
				aux_ptr_priv.input.vendor,
				aux_ptr_priv.input.model);
			free_irq(aux_ptr_irq, NULL);
			aux_ptr = 0;
		}
		aux_cmd(AUX_CMD_PTR_DISABLE);
	}
	KRN_DEBUG(0, "pointer port probed.");

	if (aux_kbd) {

		aux_cmd(AUX_CMD_KBD_ENABLE);

		aux_kbd_send(DEV_CMD_RESET);
		res = aux_in();
		if (res != DEV_BAT_SUCCESS) {

			KRN_DEBUG(1,"Unknown BAT response (%x) from kbd port",
				res);
		}
		signature =  aux_kbd_send(KBD_CMD_ID);
		signature |= aux_in() << 8;
		signature |= aux_in() << 16;

		switch (signature) {

		case SIG_POINTER:
			KRN_DEBUG(0, "pointer detected on aux_kbd port");
			aux_kbd_send(DEV_CMD_ENABLE);
			aux_kbd_parser = aux_parser_pointer;
			if (aux_kbd_focus < 0) {

				aux_kbd_focus = 0;
			}
			break;

		case SIG_KEYBOARD:
			KRN_DEBUG(0, "keyboard detected on aux_kbd port");
#if 0 /* This breaks on my machine/keyboard - Steffen_Seeger */
#warning initialize above as here!
			/*	Make the keyboard do XT translation 
			**	if the controller won't (IBM PowerPC laptops).
			*/
			aux_cmd(AUX_CMD_READ_MODE);
			res = aux_in();
			if (! (res & AUX_MODE_XT_SCANCODES)) {

				KRN_DEBUG(0, "setting XT Scancodes");
				aux_kbd_send(KBD_CMD_SCANCODE);
       				aux_kbd_send(KBD_SC_1);
			}
#endif
			aux_kbd_send(DEV_CMD_ENABLE);
			aux_kbd_parser = aux_parser_keyboard;
			if (aux_kbd_focus < 0) {

				aux_kbd_focus = 0;
			}
			break;

		default:
			KRN_DEBUG(0, "unknown signature %.6x from kbd",
				signature);
			aux_cmd(AUX_CMD_KBD_DISABLE);
			aux_cmd(AUX_CMD_WRITE_MODE);
			aux_out(AUX_MODE_XT_SCANCODES);
			free_irq(aux_kbd_irq, &aux_kbd_priv);
			aux_kbd = 0;
		}

		if (aux_kbd && aux_init_input(&aux_kbd_priv, signature,
			"kbd", aux_kbd_focus)) {

			KRN_DEBUG(0, "%s %s registration failed",
				aux_kbd_priv.input.vendor,
				aux_kbd_priv.input.model);
			free_irq(aux_ptr_irq, &aux_ptr_priv);
			aux_ptr = 0;
			aux_kbd_parser = NULL;
		}
	}
	KRN_DEBUG(0, "keyboard port probed.");

	if (! (aux_ptr || aux_kbd)) {

		KRN_ERROR("not loaded: no devices");
		io_free_region(&aux_io_region);
		return -ENODEV;
	}

	if (aux_kbd) {

		aux_cmd(AUX_CMD_KBD_ENABLE);
		aux_kbd_parser_priv = &aux_kbd_priv;
	}
	if (aux_ptr) {

		aux_cmd(AUX_CMD_PTR_ENABLE);
		aux_ptr_parser_priv = &aux_ptr_priv;
	}
	aux_cmd(AUX_CMD_WRITE_MODE);
	aux_out(AUX_MODE_XT_SCANCODES | AUX_MODE_SYSFLAG |
		(aux_kbd ? AUX_MODE_KBD_INTS_ON : 0) |
		(aux_ptr ? AUX_MODE_PTR_INTS_ON : 0));

	return KII_EOK;
}

#ifdef	MODULE
void cleanup_module(void)
{
	aux_cmd(AUX_CMD_WRITE_MODE);
	aux_out(AUX_CMD_XT_SCANCODES);

	if (aux_kbd) {

		kii_unregister_input(&aux_kbd_priv.input);
		free_irq(aux_kbd_irq, &aux_kbd_priv);
	}
	if (aux_ptr) {

		kii_unregister_input(&aux_ptr_priv.input);
		free_irq(aux_ptr_irq, &aux_ptr_priv);
	}
	io_free_region(&aux_io_region);
	KRN_DEBUG(0, "driver unloaded");
}
#else	/* #ifdef MODULE	*/

int focus_init(void)
{
	int keyboards = 0;

	if (aux_init_devices() == KII_EOK) {

		if (aux_kbd_priv.input.events & KII_EM_KEY) {

			keyboards++;
		}
		if (aux_ptr_priv.input.events & KII_EM_KEY) {

			keyboards++;
		}
	}
	printk("%i keyboards detected\n", keyboards);

	return keyboards;
}

#endif	/* #ifdef MODULE	*/

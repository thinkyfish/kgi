/*-
 * Copyright (c) 1995-1996 Andreas Beck
 * Copyright (c) 1995-2000 Steffen Seeger
 * Copyright (c) 2002-2003 Nicholas Souchu
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
 * KII manager interface definitions.
 */

#ifndef _KII_KII_H
#define _KII_KII_H

#include <dev/kgi/system.h>

KGI_SYS_DECLARE_INTEGRAL_TYPES(kii)

typedef struct { 
	kii_s_t x,y;
} kii_scoord_t;

typedef struct {
	kii_u8_t major, minor, patch, extra;
} kii_version_t;

#include <dev/kii/kiierr.h>

#ifdef KII_NEED_MODIFIER_KEYSYMS
#include <dev/kii/kiisyms.h>
#endif

typedef enum {
	KII_IC_NULL = 0,

	KII_IC_KEYBOARD = 0,		/* Character input devices.	*/
		KII_IT_KEYBOARD,	/* A simple keyboard.		*/
		KII_IT_VOICE,		/* Voice recognition. 		*/

	KII_IC_2DPOINTER = 0x10000000,	/* Simple 2D pointing devices.	*/
		KII_IT_MOUSE,		/* All kinds of mice.		*/
		KII_IT_TRACKBALL,	/* A mouse on it's back. ;-)	*/
		KII_IT_DIGITIZER,	/* Digitizer boards with pen.	*/
		KII_IT_LIGHTPEN,	/* Are there any left?		*/
		KII_IT_JOYSTICK,	/* The thing used for gaming.	*/

	KII_IC_3DPOINTER = 0x20000000,	/* 3D pointing devices.	 	*/
		KII_IT_3DMOUSE,

	KII_IC_CONTROLLER = 0x30000000,	/* Any other stuff.		*/
		KII_IT_DATAGLOVE,
		KII_IT_VR_HELMET,

	KII_IC_KEYPAD = 0x40000000,	/* Character input but no kbd.	*/

	KII_IC_MASK = 0x70000000	/* Mask to get class.		*/
} kii_type_t;

typedef enum {
	KII_EV_NOTHING	= 0x00000000,
	KII_EV_COMMAND,
	KII_EV_BROADCAST,
	KII_EV_DEVICE_INFO,

	KII_EV_RAW_DATA,

	KII_EV_KEY_PRESS,
	KII_EV_KEY_RELEASE,
	KII_EV_KEY_REPEAT,
	KII_EV_KEY_STATE,

	KII_EV_PTR_RELATIVE,
	KII_EV_PTR_ABSOLUTE,
	KII_EV_PTR_BUTTON_PRESS,
	KII_EV_PTR_BUTTON_RELEASE,
	KII_EV_PTR_STATE,

	KII_EV_VAL_RELATIVE,
	KII_EV_VAL_ABSOLUTE,
	KII_EV_VAL_STATE,

	KII_EV_LAST
} kii_event_type_t;


typedef enum {
	KII_KBD_KEYMAP0		= 0x00000000,
	KII_KBD_KEYMAP_FIRST	= 0x00000000,
	KII_KBD_KEYMAP_LAST	= 0x000000FF,
	KII_KBD_MIN_KEYCODE	= 0x00000100,
	KII_KBD_MAX_KEYCODE	= 0x00000102,
	KII_KBD_MAX_MAPSIZE	= 0x00000103,
	KII_KBD_MAPSIZE		= 0x00000104,
	KII_KBD_CLICK		= 0x00000105,
	KII_KBD_BELL_PITCH	= 0x00000106,
	KII_KBD_BELL_DURATION	= 0x00000107,
	KII_KBD_AUTO_RATE	= 0x00000108,
	KII_KBD_AUTO_DELAY	= 0x00000109,
	KII_KBD_LEDS		= 0x0000010A,

	KII_PTR_BUTTON0		= 0x00000110,
	KII_PTR_ACCEL_MUL	= 0x00000120,
	KII_PTR_ACCEL_DIV	= 0x00000121,
	KII_PTR_ACCEL_TRESHOLD	= 0x00000122,

	KII_LAST
} kii_enum_t;

typedef enum {
	/* Event masks. */
	KII_EM_NOTHING			= (1 << KII_EV_NOTHING),
	KII_EM_COMMAND			= (1 << KII_EV_COMMAND),
	KII_EM_BROADCAST		= (1 << KII_EV_BROADCAST),
	KII_EM_DEVICE_INFO		= (1 << KII_EV_DEVICE_INFO),

	KII_EM_RAW_DATA			= (1 << KII_EV_RAW_DATA),

	KII_EM_KEY_PRESS		= (1 << KII_EV_KEY_PRESS),
	KII_EM_KEY_RELEASE		= (1 << KII_EV_KEY_RELEASE),
	KII_EM_KEY_REPEAT		= (1 << KII_EV_KEY_REPEAT),
	KII_EM_KEY_STATE		= (1 << KII_EV_KEY_STATE),

	KII_EM_PTR_RELATIVE		= (1 << KII_EV_PTR_RELATIVE),
	KII_EM_PTR_ABSOLUTE		= (1 << KII_EV_PTR_ABSOLUTE),
	KII_EM_PTR_BUTTON_PRESS		= (1 << KII_EV_PTR_BUTTON_PRESS),
	KII_EM_PTR_BUTTON_RELEASE	= (1 << KII_EV_PTR_BUTTON_RELEASE),
	KII_EM_PTR_STATE		= (1 << KII_EV_PTR_STATE),

	KII_EM_VAL_RELATIVE		= (1 << KII_EV_VAL_RELATIVE),
	KII_EM_VAL_ABSOLUTE		= (1 << KII_EV_VAL_ABSOLUTE),
	KII_EM_VAL_STATE		= (1 << KII_EV_VAL_STATE),

	KII_EM_KEY	= KII_EM_KEY_PRESS | KII_EM_KEY_RELEASE |
		KII_EM_KEY_REPEAT,
	KII_EM_KEYBOARD	= KII_EM_KEY | KII_EM_KEY_STATE,

	KII_EM_PTR_MOVE	= KII_EM_PTR_RELATIVE | KII_EM_PTR_ABSOLUTE,
	KII_EM_PTR_BUTTON = KII_EM_PTR_BUTTON_PRESS | KII_EM_PTR_BUTTON_RELEASE,
	KII_EM_POINTER	= KII_EM_PTR_MOVE | KII_EM_PTR_BUTTON | 
		KII_EM_PTR_STATE,

	KII_EM_VALUATOR	= KII_EM_VAL_RELATIVE | KII_EM_VAL_ABSOLUTE |
		KII_EM_VAL_STATE,

	KII_EM_ALL	= ((1 << KII_EV_LAST) - 1) & ~KII_EM_NOTHING
} kii_event_mask_t;


#define	KII_KBD_KEYMAP(n)	(KII_KBD_KEYMAP0 + (n))
#define	KII_PTR_BUTTON(n)	(KII_PTR_BUTTON0 + (n))

/*
 * There may be up to 16 modifiers. You don't have more then ten fingers
 * anyway.
 */
#define	KII_MODIFIER(sym)		((sym) & 15)

#define	KII_IS_NORMAL_MODIFIER(sym)	(((sym) & ~15) == K_FIRST_NORMAL)
#define	KII_IS_LOCKED_MODIFIER(sym)	(((sym) & ~15) == K_FIRST_LOCKED)
#define	KII_IS_STICKY_MODIFIER(sym)	(((sym) & ~15) == K_FIRST_STICKY)

#define	KII_IS_MODIFIER(sym)		(KII_IS_NORMAL_MODIFIER(sym) || \
					 KII_IS_LOCKED_MODIFIER(sym) || \
					 KII_IS_STICKY_MODIFIER(sym))




/*
 * The event structs.
 * 
 *  NOTE We depend on __KII_MODIFIER_DATA being declared _directly_ after
 * __KII_COMMON_DATA.
 */
#define	__KII_MODIFIER_DATA\
	kii_u16_t	effect;		/* current modifiers in effect	*/\
	kii_u16_t	normal;		/* normal modifiers		*/\
	kii_u16_t	locked;		/* locked modifiers		*/\
	kii_u16_t	sticky		/* sticky modifiers		*/

#define	__KII_COMMON_DATA\
	kii_u8_t	size;		/* size of event in bytes	*/\
	kii_u8_t	type;		/* type of this event		*/\
	kii_u8_t	focus;		/* focus this is reported from	*/\
	kii_u8_t	device;		/* who sent this		*/\
	kii_u32_t	time;		/* timestamp			*/\
	kii_u_t		dontdispatch	/* 1 if no put_event            */

/*
 * This information is reported with all events. Use the <any> field
 * in a kii_event structure to access these fields.
 */ 
typedef struct {
	__KII_COMMON_DATA;
} kii_any_event_t;

/*
 * The recipient must not store references to the data.
 * If the data information is needed afterwards, copy it!
 */
typedef struct {
	__KII_COMMON_DATA;

	kii_u32_t	code;		/* Command/request code.		*/
	kii_u8_t	data[1];	/* Command related data.		*/
} kii_cmd_event_t;

/*
 * This reports a block of raw data received from the device. The
 * maximum number of bytes is limited so that the event struct is
 * not more than 256 bytes.
 */
typedef struct {
	__KII_COMMON_DATA;

	kii_u8_t	data[255 - sizeof(kii_any_event_t)];
} kii_raw_event_t;

/*
 * Key events should be used to report events obtained from keys
 * and other switches. 
 */
typedef struct {
	__KII_COMMON_DATA;

	__KII_MODIFIER_DATA;
	kii_u32_t	sym;		/* Action(effective, keycode)	*/
	kii_u32_t	code;		/* Key pressed.			*/
} kii_key_event_t;

/*
 * This is used to report change of pointer position. Depending
 * on the event type, the values are either absolute or relative.
 */
typedef struct {
	__KII_COMMON_DATA;

	__KII_MODIFIER_DATA;	/* Current state of modifiers.*/
	kii_s32_t	x, y;		/* Absolute/relative position.*/
	kii_s32_t	wheel;		/* Absolute/relative wheel.	*/

	kii_u32_t	dummy[2];	/* Reserved (future extensions)	*/ 
} kii_pmove_event_t;

/*
 * Button events are sent to report a change in pointer button state: 
 * Bit#0 = primary button (usually left)
 * Bit#1 = secondary button (usually right)
 * Bit#2 = tertiary buttin (usually middle)
 * All further button in some "logical" ordering from then on.
 */
typedef struct {
	__KII_COMMON_DATA;

	__KII_MODIFIER_DATA;	/* Current state of modifiers.	*/
	kii_u32_t	state;		/* New button state.		*/
	kii_u32_t	button;		/* Button(s) that caused event.	*/
} kii_pbutton_event_t;

/*
 * Valuator events report a change of up to 32 of the input device
 * valuators. Only a range of 32 valuators beginning from first
 * can be reported with one event. For performance reasons, only
 * changed values are reported in value[] thus you can't use
 * (num - first) as index to get info for valuator (num).
 */
typedef struct {
	__KII_COMMON_DATA;

	kii_u32_t	first;		/* First valuator reported.	*/
	kii_u32_t	changed;	/* Valuators that are reported.	*/
	kii_u32_t	value[32];	/* Absolute/relative values.	*/
} kii_val_event_t;

typedef union {
	kii_u8_t		size;		/* Size of this event.	*/

	kii_any_event_t		any;		/* Access COMMON_DATA.	*/
	kii_cmd_event_t		cmd;		/* Command/broadcast.	*/
	kii_raw_event_t		raw;		/* Raw data event.	*/
	kii_val_event_t		val;		/* Valuator change.	*/
	kii_key_event_t		key;		/* Key press/release.	*/
	kii_pmove_event_t	pmove;		/* Pointer move.		*/
	kii_pbutton_event_t	pbutton;	/* Pointer buttons.	*/
} kii_event_t;

#ifdef	_KERNEL

#define	KII_MAX_KEYMAP_MEMORY	32768	/* bytes			*/
#define	KII_MAX_NR_FOCUSES	CONFIG_KGII_MAX_NR_FOCUSES
#define	KII_MAX_NR_CONSOLES	CONFIG_KGII_MAX_NR_CONSOLES
#define	KII_MAX_NR_DEVICES	CONFIG_KGII_MAX_NR_DEVICES
#define	KII_MAX_NR_INPUTS	CONFIG_KGII_MAX_NR_INPUTS

#define	KII_INVALID_FOCUS	INVALID_FOCUS
#define	KII_INVALID_CONSOLE	INVALID_CONSOLE
#define	KII_INVALID_INPUT	INVALID_INPUT
#define	KII_INVALID_DEVICE	INVALID_DEVICE

typedef struct kii_input_s kii_input_t;

#define KII_MAX_VENDOR_STRING	64
#define KII_MAX_MODEL_STRING	64

struct kii_input_s {
	kii_input_t		*next;	/* Next in linked list.		*/
	kii_u_t			focus;	/* The focus we belong to.	*/
	kii_u_t			id;	/* Unique input device ID.	*/
	kii_event_mask_t	events;	/* Event types generated.	*/
	kii_event_mask_t	report;	/* Events to report.		*/

	kii_ascii_t	vendor[KII_MAX_VENDOR_STRING];	/* Who built this device.	*/
	kii_ascii_t	model[KII_MAX_MODEL_STRING];	/* What model.			*/

	int	(*Command)(kii_input_t *, kii_u_t cmd, void *);
	int	(*Poll)(kii_input_t *);
	void	(*Parse)(kii_input_t *, kii_event_t *, int);

	kii_private_t	priv;
};

#define	KII_VALID_FOCUS_ID(x)	((x) < KII_MAX_NR_FOCUSES)
#define	KII_VALID_CONSOLE_ID(x)	((x) < KII_MAX_NR_CONSOLES)
#define	KII_VALID_INPUT_ID(x)	((x) < KII_MAX_NR_INPUTS)
#define	KII_VALID_DEVICE_ID(x)	((x) < KII_MAX_NR_DEVICES)

typedef struct {
	kii_unicode_t	diacr;
	kii_unicode_t	base;
	kii_unicode_t	combined;
} kii_dead_combination_t;

typedef struct {
	kii_u_t	 fn_buf_size;	/* Size of fn_key string buffer.	*/
	kii_ascii_t *fn_buf;	/* fn_key string buffer.		*/

	kii_u_t	fn_str_size;	/* Size of function key string buffer.	*/
	kii_ascii_t **fn_str;	/* ptr to arr of ptrs to strings.	*/

	kii_u_t	keymin, keymax;	/* Min/Max key values accepted.		*/
	kii_u_t	keymap_size;	/* Size of keymap array.			*/
	kii_unicode_t **keymap;	/* The keymaps.				*/

	kii_u_t	combine_size;
	kii_dead_combination_t *combine;
} kii_keymap_t;

typedef enum {
	KII_FF_SCROLL_LOCK		= 0x00000001,
	KII_FF_CAPS_SHIFT		= 0x00000002,
	KII_FF_NUM_LOCK			= 0x00000004,
	KII_FF_LED_FLAGS		= 0x000000FF,
	KII_FF_HAS_POINTER		= 0x00000100,
	KII_FF_PROCESS_BH		= 0x00000200,
	KII_FF_SYSTEM_REQUEST	= 0x00000400
} kii_focus_flags_t;

typedef enum {
	KII_DF_SCROLL_LOCK	= 0x00000001,
	KII_DF_CAPS_SHIFT	= 0x00000002,
	KII_DF_NUM_LOCK		= 0x00000004,
	KII_DF_LED_FLAGS	= 0x000000FF,
	KII_DF_FOCUSED		= 0x00000100,
	KII_DF_CONSOLE		= 0x00000200,
	KII_DF_CLONED		= 0x00000400
} kii_device_flags_t;

typedef struct kii_device_s kii_device_t;

struct kii_device_s {
	kii_u_t			id;
	kii_u_t			focus_id;
	void			*tty;
	kii_device_flags_t	flags;
	kii_event_mask_t	event_mask;
	kii_private_t		priv;

	void	(*MapDevice)(kii_device_t *);
	kii_s_t	(*UnmapDevice)(kii_device_t *);

	void (*HandleEvent)(kii_device_t *, kii_event_t *);

	kii_scoord_t	ptr;
	kii_scoord_t	ptr_min;
	kii_scoord_t	ptr_max;

	kii_private_t	spawnpid;
	kii_private_t	spawnsig;
};

typedef struct {
	kii_u_t		id;		/* Unique focus ID.		*/
	kii_device_t	*focus;		/* Focused device.		*/
	kii_u_t		curr_console;	/* Current device.		*/
	kii_u_t		last_console;	/* Last device focused on.	*/
	kii_u_t		want_console;	/* Device to focus on.		*/

	kii_input_t	*inputs;	/* Input devices.		*/

	__KII_MODIFIER_DATA;		/* Modifier information.		*/
	kii_u_t		down_mod[16];	/* Modifier counters.		*/
	kii_keymap_t	kmap;		/* Key map info.			*/

	kii_focus_flags_t flags;	/* Focus-global flags.		*/

	kii_u_t		ptr_buttons;	/* Pointer button state.		*/
	kii_scoord_t	ptr;		/* Pointer hot spot position.	*/
	kii_scoord_t	ptr_max;	/* Pointer hot spot window.	*/
	kii_scoord_t	ptr_min;	/* Pointer hot spot window.	*/

	kii_unicode_t	npadch;		/* keysym assembled on keypad.	*/
	kii_unicode_t	dead;		/* Pending dead/compose keysym.	*/
	void		*pt_regs;	/* Argument for show_regs.	*/

	kii_u_t		next_input_id;
	kii_u8_t	*console_map;
	kii_u8_t	*graphic_map;
	kii_u8_t	focus_graph[KII_MAX_NR_CONSOLES];
} kii_focus_t;

typedef enum {
	KII_SYNC_LED_FLAGS    = 0x00000001,
	KII_SYNC_PTR_POSITION = 0x00000002,
	KII_SYNC_PTR_WINDOW	  = 0x00000004,
	KII_SYNC_PTR_BUTTONS  = 0x00000008
} kii_sync_flags_t;



extern kii_device_t *kiidevice[KII_MAX_NR_DEVICES];
extern kii_focus_t  *kiifocus[KII_MAX_NR_FOCUSES];

extern void kii_bottomhalf(void);

/* XXX currently implemented by kip. */
extern void kii_handle_input(kii_event_t *event);

extern void kii_action(kii_focus_t *f, kii_event_t *event);
extern void kii_put_event(kii_focus_t *f, kii_event_t *event);

extern kii_error_t kii_register_input(kii_u_t focus, kii_input_t *dev, int reset);
extern void kii_unregister_input(kii_input_t *dev);

extern kii_error_t kii_register_device(kii_device_t *dev, kii_u_t index);
extern void kii_unregister_device(kii_device_t *dev);

extern kii_device_t *kii_clone_console(kii_u_t source, kii_u_t target);
extern kii_error_t kii_delete_clone(kii_u_t dev_id);

extern void kii_map_device(kii_u_t dev_id);
extern kii_s_t kii_unmap_device(kii_u_t dev_id);

/*
 * The proper prototype is int kii_focus_of_task(struct task_struct *);
 * However, including the scheduler is not desireable.
 */
extern kii_s_t kii_focus_of_task(void *);
extern kii_device_t *kii_current_focus(kii_u_t focus_id);
extern kii_s_t kii_console_device (kii_s_t focus_id);
extern void kii_poll_device(kii_u_t dev_id, kii_event_t *event);

extern const kii_ascii_t *kiidev_get_fnstring(kii_device_t *dev, kii_u_t key);
extern kii_focus_t *kiidev_focus(kii_s_t dev_id);
extern void kiidev_sync(kii_device_t *dev, kii_sync_flags_t what);
extern void kiidev_set_pointer_window(kii_device_t *dev, 
	kii_s_t minx, kii_s_t maxx, kii_s_t miny, kii_s_t maxy);

extern kii_unicode_t keymap_get_keysym(kii_keymap_t *k, kii_u_t shift, kii_u_t key);
extern kii_s_t keymap_set_keysym(kii_keymap_t *k, kii_u_t shift, kii_u_t key, kii_unicode_t sym);
extern kii_s_t keymap_set_default(kii_u_t shift, kii_u_t key, kii_unicode_t sym);
extern const kii_ascii_t *keymap_get_fnstring(kii_keymap_t *k, kii_u_t key);
extern kii_error_t keymap_set_fnstring(kii_keymap_t *k, kii_u_t key, const kii_ascii_t *s);

extern kii_keymap_t default_kii_keymap;
extern void keymap_reset(kii_keymap_t *k);
extern kii_unicode_t keymap_toggled_case(kii_unicode_t sym);
extern kii_unicode_t keymap_combine_dead(kii_keymap_t *k,
					 kii_unicode_t diacr, kii_unicode_t base);
extern kii_s_t keymap_set_default_keysym(kii_u_t shift, kii_u_t key,
					 kii_unicode_t sym);

#endif /* #ifdef _KERNEL */

#include <dev/kii/kiicmd.h>

#endif /* #ifdef _KII_KII_H */

/* ----------------------------------------------------------------------------
**	KII input manager
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
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
**	$Log: kii.c,v $
**	Revision 1.2  2000/09/21 09:12:40  seeger_s
**	- minor bugfixes
**	- public symbols exported
**	
**	Revision 1.1.1.1  2000/04/18 08:50:53  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	DEBUG_LEVEL	1

#include <linux/sched.h>	/* declares jiffies counter */
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/kgii.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
#define KEYBOARD_BH CONSOLE_BH
#endif

extern void ctrl_alt_del(void);
extern int vt_dont_switch;

/*
**	input device administration
*/
static kii_focus_t  *kiifocus[KII_MAX_NR_FOCUSES];
kii_device_t *kiidevice[KII_MAX_NR_DEVICES];

/*
**	keymap lookup stuff
*/

static void keymap_reset(kii_keymap_t *k)
{
	kii_u_t i;

	if (k && (k != &default_kii_keymap)) {

#		define	FREE(x)					\
			if (k->x != default_kii_keymap.x) {	\
								\
				kfree(k->x);			\
				KRN_TRACE(0, k->x = NULL);	\
			}

		FREE(fn_buf)
		FREE(fn_str)
		for (i = 0; i < k->keymap_size; i++) {

			FREE(keymap[i]);
		}
		FREE(keymap)

		memcpy(k, &default_kii_keymap, sizeof(*k));
#		undef 	FREE
	}
}

kii_s_t keymap_set_keysym(kii_keymap_t *k, kii_u_t shift, kii_u_t key,
	kii_unicode_t sym)
{
	kii_unicode_t *newmap;
	kii_u_t i, size;

	KRN_ASSERT(k);

	if ((k == NULL) || (k->keymap_size <= shift) || (key < k->keymin) ||
		(k->keymax < key)) {

		return -EINVAL;
	}
	if (k->keymap[shift] &&
		(k->keymap[shift] != default_kii_keymap.keymap[shift])) {

		k->keymap[shift][key] = sym;
		return KII_EOK;
	}

	if ((sym == K_VOID) && (k->keymap[shift] == NULL)) {

		return KII_EOK;
	}

	size = sizeof(kii_keymap_t) + k->fn_buf_size;
	for (i = 0; i < k->keymap_size; i++) {

		if (k->keymap[i]) {

			size += sizeof(kii_unicode_t) * (k->keymax-k->keymin+1);
		}
	}
	size += sizeof(void *) * (k->fn_str_size + k->keymap_size);
	size += sizeof(kii_dead_combination_t) * k->combine_size;
	size += sizeof(kii_unicode_t) * (k->keymax-k->keymin+1);
	if (KII_MAX_KEYMAP_MEMORY < size) {

		return -ENOMEM;
	}

	if (k->keymap == default_kii_keymap.keymap) {

		kii_unicode_t **newarr;

		size = sizeof(k->keymap[0]) * k->keymap_size;
		if (! (newarr = kmalloc(size, GFP_USER))) {

			return -ENOMEM;
		}
		memcpy(newarr, k->keymap, size);
		k->keymap = newarr;
		KRN_DEBUG(2, "allocated new keymap array (%i slots)", 
			k->keymap_size);
	}
	size = k->keymax - k->keymin + 1;
	if (!(newmap = kmalloc(sizeof(kii_unicode_t) * size, GFP_USER))) {

		return -ENOMEM;
	}
	KRN_DEBUG(2, "%s keymap %i", 
		default_kii_keymap.keymap[shift] ? "reallocated" : "allocated",
		shift);
	i = size;
	if (default_kii_keymap.keymap[shift]) {

		while (i--) {

			newmap[i] = default_kii_keymap.keymap[shift][i];
		}

	} else {

		while (i--) {

			newmap[i] = K_VOID;
		}
	}
	newmap[key] = sym;
	k->keymap[shift] = newmap;
	KRN_ASSERT(k->keymap[shift] != default_kii_keymap.keymap[shift]);
	return KII_EOK;
}

kii_s_t keymap_set_default_keysym(kii_u_t shift, kii_u_t key,
	kii_unicode_t sym)
{
	kii_keymap_t *k = &default_kii_keymap;
	kii_unicode_t *newmap;
	kii_u_t i;

	if ((k->keymap_size <= shift) || (key < k->keymin) ||
		(k->keymax < key)) {

		return -EINVAL;
	}
	if (k->keymap[shift]) {

		k->keymap[shift][key] = sym;
		return KII_EOK;
	}

	if (sym == K_VOID) {

		return KII_EOK;
	}

	newmap = kmalloc((k->keymax - k->keymin + 1) *
		sizeof(kii_unicode_t), GFP_KERNEL);
	if (newmap == NULL) {

		return -ENOMEM;
	}
	for (i = k->keymax - k->keymin; i; i--) {

		newmap[i] = K_VOID;
	}
	newmap[key] = sym;
	k->keymap[shift] = newmap;
	return KII_EOK;
}


inline kii_unicode_t keymap_get_keysym(kii_keymap_t *k, kii_u_t shift,
	kii_u_t key)
{
	kii_unicode_t *map, sym;

	if ((key < k->keymin) || (k->keymax < key)) {

		return K_VOID;
	}

	key -= k->keymin;
	map = (shift < k->keymap_size) ? k->keymap[shift] : NULL;
	sym = map ? map[key] : K_VOID;

	if ((k->keymap[0][key] & K_TYPE_MASK) == K_TYPE_SHIFT) {

		sym = k->keymap[0][key];
	}
	return sym;
}


static inline kii_unicode_t keymap_combine_dead(kii_keymap_t *k,
	kii_unicode_t diacr, kii_unicode_t base)
{
	kii_u_t i;

	for (i = 0; i < k->combine_size; i++) {

		if ((k->combine[i].diacr == diacr) && 
			(k->combine[i].base == base)) {

			return k->combine[i].combined;
		}
	}
	switch (K_TYPE(base)) {

	case K_TYPE_SPECIAL:
	case K_TYPE_SHIFT:
		return K_VOID;
	default:
		return diacr;
	}
}


const kii_ascii_t *keymap_get_fnstring(kii_keymap_t *k, kii_u_t key)
{
	return (key < k->fn_str_size) ? k->fn_str[key] : NULL;
}

kii_s_t keymap_set_fnstring(kii_keymap_t *k, kii_u_t key, const kii_ascii_t *s)
{
	KRN_DEBUG(2, "keymap_set_fnstring() not implemented yet!");
	return -EINVAL;
}


struct __kii_keymap_translation_t { kii_unicode_t val1, val2, xor; };

static const struct __kii_keymap_translation_t keymap_range_translation[] =
{
	{ 0x0041, 0x005a,  0x0020 }, { 0x0061, 0x007a,  0x0020 },
	{ 0x00c0, 0x00de,  0x0020 }, { 0x00e0, 0x00fe,  0x0020 },
	{ 0x0100, 0x012f,  0x0001 }, { 0x0132, 0x0137,  0x0001 },
	{ 0x014a, 0x0177,  0x0001 }, { 0x0182, 0x0185,  0x0001 },
	{ 0x01a0, 0x01a5,  0x0001 }, { 0x01e0, 0x01ef,  0x0001 },
	{ 0x01fa, 0x01ff,  0x0001 }, { 0x0200, 0x0217,  0x0001 },
	{ 0x0391, 0x039f,  0x0020 }, { 0x03a0, 0x03ab,  0x0060 },
	{ 0x03b1, 0x03bf,  0x0020 }, { 0x03c0, 0x03cb,  0x0060 },
	{ 0x03e2, 0x03ef,  0x0001 }, { 0x0401, 0x040f,  0x0050 },
	{ 0x0410, 0x041f,  0x0020 }, { 0x0420, 0x042f,  0x0060 },
	{ 0x0430, 0x043f,  0x0020 }, { 0x0440, 0x044f,  0x0060 },
	{ 0x0451, 0x045f,  0x0050 }, { 0x0460, 0x0481,  0x0001 },
	{ 0x0490, 0x04bf,  0x0001 }, { 0x04d0, 0x04f9,  0x0001 },
	{ 0x0531, 0x053f,  0x0050 }, { 0x0540, 0x054f,  0x0030 },
	{ 0x0550, 0x0556,  0x00d0 }, { 0x0561, 0x056f,  0x0050 },
	{ 0x0570, 0x057f,  0x0030 }, { 0x0580, 0x0586,  0x00d0 },
	{ 0x1e00, 0x1e59,  0x0001 }, { 0x1ea0, 0x1ef9,  0x0001 },
	{ 0x1f00, 0x1f15,  0x0008 }, { 0x1f18, 0x1f1d,  0x0008 },
	{ 0x1f20, 0x1f45,  0x0008 }, { 0x1f48, 0x1f4d,  0x0008 },
	{ 0x1f68, 0x1f60,  0x0008 }, { 0xff21, 0xff3a,  0x0060 },
	{ 0xff41, 0xff5a,  0x0060 }
};

static const struct __kii_keymap_translation_t keymap_pair_translation[] =
{
	{ 0x0139, 0x013a,  0x0003 }, { 0x013b, 0x013c,  0x0007 },
	{ 0x013d, 0x013e,  0x0003 }, { 0x013f, 0x0140,  0x007f },
	{ 0x0141, 0x0142,  0x0003 }, { 0x0143, 0x0144,  0x0007 },
	{ 0x0145, 0x0146,  0x0003 }, { 0x0147, 0x0148,  0x000f },
	{ 0x0178, 0x00ff,  0x0187 }, { 0x0179, 0x017a,  0x0003 },
	{ 0x017b, 0x017c,  0x0007 }, { 0x017d, 0x017e,  0x0003 },
	{ 0x0181, 0x0253,  0x03d2 }, { 0x0186, 0x0254,  0x03d2 },
	{ 0x0187, 0x0188,  0x000f }, { 0x018a, 0x0257,  0x03dd },
	{ 0x018b, 0x018c,  0x0007 }, { 0x018e, 0x0258,  0x03d6 },
	{ 0x018f, 0x0259,  0x03d6 }, { 0x0190, 0x025b,  0x03cb },
	{ 0x0191, 0x0192,  0x0003 }, { 0x0193, 0x0260,  0x03f3 },
	{ 0x0194, 0x0263,  0x03f7 }, { 0x0196, 0x0269,  0x03ff },
	{ 0x0197, 0x0268,  0x03ff }, { 0x0198, 0x0199,  0x0001 },
	{ 0x019c, 0x026f,  0x03f3 }, { 0x019d, 0x0272,  0x03ef },
	{ 0x01a7, 0x01a8,  0x000f }, { 0x01a9, 0x0283,  0x032a },
	{ 0x01ac, 0x01ad,  0x0001 }, { 0x01ae, 0x0288,  0x0326 },
	{ 0x01af, 0x01b0,  0x001f }, { 0x01b1, 0x028a,  0x033b },
	{ 0x01b2, 0x028b,  0x0339 }, { 0x01b3, 0x01b4,  0x0007 },
	{ 0x01b5, 0x01b6,  0x0003 }, { 0x01b7, 0x0292,  0x0325 },
	{ 0x01b8, 0x01b9,  0x0001 }, { 0x01bc, 0x01bd,  0x0001 },
	{ 0x01c4, 0x01c6,  0x0002 }, { 0x01c7, 0x01c9,  0x000e },
	{ 0x01ca, 0x01cc,  0x0006 }, { 0x01cd, 0x01ce,  0x0003 },
	{ 0x01cf, 0x01d0,  0x001f }, { 0x01d1, 0x01d2,  0x0003 },
	{ 0x01d3, 0x01d4,  0x0007 }, { 0x01d5, 0x01d6,  0x0003 },
	{ 0x01d7, 0x01d8,  0x000f }, { 0x01d9, 0x01da,  0x0003 },
	{ 0x01db, 0x01dc,  0x0007 }, { 0x01de, 0x01df,  0x0001 },
	{ 0x01f1, 0x01f3,  0x0002 }, { 0x01f4, 0x01f5,  0x0001 },
	{ 0x0386, 0x03ac,  0x002a }, { 0x0388, 0x03ad,  0x0025 },
	{ 0x0389, 0x03ae,  0x0027 }, { 0x038a, 0x03af,  0x0025 },
	{ 0x038c, 0x03cc,  0x0040 }, { 0x038e, 0x03cd,  0x0043 },
	{ 0x038f, 0x03ce,  0x0041 }, { 0x04c1, 0x04c2,  0x0003 },
	{ 0x04c3, 0x04c4,  0x0007 }, { 0x04c7, 0x04c8,  0x000f },
	{ 0x04cb, 0x04cc,  0x0007 }, { 0x1f59, 0x1f51,  0x0008 },
	{ 0x1f5b, 0x1f53,  0x0008 }, { 0x1f5d, 0x1f55,  0x0008 },
	{ 0x1f5f, 0x1f57,  0x0008 }, { 0x1fba, 0x1f70,  0x00ca },
	{ 0x1fbb, 0x1f71,  0x00ca }, { 0x1fc8, 0x1f72,  0x00ba },
	{ 0x1fc9, 0x1f73,  0x00ba }, { 0x1fca, 0x1f74,  0x00be },
	{ 0x1fcb, 0x1f75,  0x00be }, { 0x1fda, 0x1f76,  0x00ac },
	{ 0x1fdb, 0x1f77,  0x00ac }, { 0x1fea, 0x1f7a,  0x0090 },
	{ 0x1feb, 0x1f7b,  0x0090 }, { 0x1fec, 0x1fe5,  0x0009 },
	{ 0x1ff8, 0x1f78,  0x0080 }, { 0x1ff9, 0x1f79,  0x0080 },
	{ 0x1ffa, 0x1f7c,  0x0086 }, { 0x1ffb, 0x1f7d,  0x0086 },
	{ 0x2112, 0x2113,  0x0001 }, { 0x2130, 0x212f,  0x001f }
};

static inline kii_unicode_t keymap_toggled_case(kii_unicode_t sym)
{
	kii_u_t i, size;

	size = sizeof(keymap_pair_translation) /
		sizeof(keymap_pair_translation[0]);

	for (i = 0; i < size; i++) {

		if ((keymap_pair_translation[i].val1 == sym) || 
			(keymap_pair_translation[i].val2 == sym)) {

			return sym ^ keymap_pair_translation[i].xor;
		}
	}

	size = sizeof(keymap_range_translation) /
		sizeof(keymap_range_translation[0]);

	for (i = 0; i < size; i++) {

		if ((keymap_range_translation[i].val1 <= sym) &&
			(sym <= keymap_range_translation[i].val2)) {

			return sym ^ keymap_range_translation[i].xor;
		}
	}

	if ((0x24b6 <= sym) && (sym <= 0x24cf)) {

		return sym + 26;
	}
	if ((0x24d0 <= sym) && (sym <= 0x24e9)) {

		return sym - 26;
	}

	return sym;
}

static kii_u_t kii_next_input_id(kii_focus_t *f)
{
	kii_u_t input_id = f->next_input_id;
	kii_input_t *input;

	do {
		if (KII_MAX_NR_INPUTS <= ++input_id) {

			input_id = 0;
		}

		input = f->inputs;
		while (input && (input->id != input_id)) {

			input = input->next;
		}
		if (input == NULL) {

			return input_id;
		}

	} while (input_id != f->next_input_id);

	return KII_INVALID_INPUT;
}

/*	Register an input device to a focus. We keep KII_IC_2DPOINTER devices
**	first in the linked list to tell easily if a has a pointer.
**	return values:
**		EOK	successful
**		-EINVAL	parameters invalid
*/
kii_s_t kii_register_input(kii_u_t focus, kii_input_t *dev)
{
	kii_focus_t *f;
	kii_input_t *prev;
	kii_u_t input_id;


	/*	Auto-assign to next focus that needs one of these.
	*/
	if (dev && (focus == KII_INVALID_FOCUS)) {

		focus = 0;
		while (KII_VALID_FOCUS_ID(focus)) {

			if (kiifocus[focus] == NULL) {

				break;
			}
			prev = (kiifocus[focus])->inputs;
			while (prev != NULL) {

				if ((dev->events & prev->events &
					KII_EM_POINTER) || (dev->events &
					prev->events & KII_EM_KEY)) {

					prev = NULL;  
					break; 
				}
				if (prev->next == NULL) {

					break;
				}
				prev = prev->next;
			}
			if (prev != NULL) {

				break;
			}
			focus++;
		} 
	}

	if (!(dev && KII_VALID_FOCUS_ID(focus))) {

		KRN_DEBUG(1, "invalid parameters: focus = %i, dev = %p",
			focus, dev);
		return -EINVAL;
	}

	f = kiifocus[focus];

	if (! f) {

		kii_s_t i;

		KRN_DEBUG(2, "allocating focus %i...", focus);
		f = kiifocus[focus] = kmalloc(sizeof(kii_focus_t), GFP_KERNEL);
		if (! f) { 

			KRN_DEBUG(2, "focus %i allocation failed", focus);
			return -ENOMEM;
		}
		memset(f, 0, sizeof(*f));
		f->id = focus;
		f->curr_console = 
		f->last_console = 
		f->want_console = KII_INVALID_CONSOLE;
		f->dead = f->npadch = K_VOID;
		f->console_map = console_map[f->id];
		f->graphic_map = graphic_map[f->id];
		keymap_reset(&f->kmap);

		i = 0;
		while ((i < KII_MAX_NR_CONSOLES) && !(
			(KII_VALID_DEVICE_ID(f->console_map[i]) &&	
				kiidevice[f->console_map[i]]) ||
			(KII_VALID_DEVICE_ID(f->graphic_map[i]) &&
				kiidevice[f->graphic_map[i]]))) { 

			i++;
		}
		if ((KII_VALID_DEVICE_ID(f->graphic_map[i]) &&
			kiidevice[f->graphic_map[i]])) {

			f->curr_console = i;
			f->focus = kiidevice[f->graphic_map[i]];
		}
		if ((KII_VALID_DEVICE_ID(f->console_map[i]) &&
			kiidevice[f->console_map[i]])) {

			f->curr_console = i;
			f->focus = kiidevice[f->console_map[i]];
		}
		if (f->focus) {

			f->focus->flags |= KII_DF_FOCUSED;
			if (f->focus->MapDevice) {

				(f->focus->MapDevice)(f->focus);
			}
		}

		KRN_DEBUG(2, "focus %i allocated, focused on console %i",
			focus, f->curr_console);
	}

	input_id = kii_next_input_id(f);
	if (input_id == KII_INVALID_INPUT) {

		KRN_DEBUG(1, "could not get valid input ID");
		return -EBUSY;
	}
	dev->id = input_id;
	dev->focus = f->id;

	if ((prev = f->inputs)) {

		while (prev->next && (prev->next->events & KII_EM_POINTER)) {

			prev = prev->next;
		}
		dev->next  = prev->next;
		prev->next = dev;

	} else {

		dev->next  = NULL;
		f->inputs  = dev;
	}

	f->flags |= (dev->events & KII_EM_POINTER) ? KII_FF_HAS_POINTER : 0;
	return KII_EOK;
}

/*	Unregister a previously registered input device from its focus.
**	If dev is not valid or dev was not registered, we do nothing.
*/
void kii_unregister_input(kii_input_t *dev)
{
	kii_input_t *prev;
	kii_focus_t *f; 

	if (! (dev && KII_VALID_FOCUS_ID(dev->focus) && 
		(f = kiifocus[dev->focus]))) {

		KRN_DEBUG(1, "invalid paramters: dev = %p, focus = %i",
			dev, dev ? dev->focus : KII_INVALID_FOCUS);
		return;
	}
	KRN_ASSERT(dev->focus == f->id);

	if (dev != f->inputs) {

		prev = f->inputs;
		while (prev && (prev->next != dev)) {

			prev = prev->next;
		}

		if (prev) {

			prev->next = dev->next;
			dev->next  = NULL;
			dev->focus = KII_INVALID_FOCUS;
			dev->id    = KII_INVALID_INPUT;

		} else {

			/*	The device is not listed in that focus. 
			**	We better report...
			*/
			KRN_DEBUG(0, "device %p ('%s %s') not in list?",
				dev, dev->vendor, dev->model);
			return;
		}

	} else {	/* if (dev != f->inputs) ... */

		f->inputs  = dev->next;
		dev->next  = NULL;
		dev->focus = KII_INVALID_FOCUS;
		dev->id    = KII_INVALID_INPUT;

		/*	Even if there are no input devices left, just to keep
		**	the focus hanging around. However, free the keymap
		**	and reset it to the default one.
		*/
		if (! f->inputs) {

			keymap_reset(&f->kmap);
		}
	}

	f->flags &= ~KII_FF_HAS_POINTER;
	f->flags |= (f->inputs && (KII_EM_POINTER & f->inputs->events)) 
			? KII_FF_HAS_POINTER : 0;
}

kii_s_t kii_register_device(kii_device_t *dev, kii_u_t index)
{
	kgi_u_t focus, console;
	kgi_u8_t *map;

	KRN_ASSERT(dev);
	if (! (dev && KII_VALID_CONSOLE_ID(index))) {

		KRN_DEBUG(1, "invalid arguments %p, %i", 
			dev, index);
		return -EINVAL;
	}
	dev->id = (dev->flags & KII_DF_CONSOLE) 
		? index : index + KII_MAX_NR_CONSOLES;

	KRN_ASSERT(sizeof(console_map) == sizeof(graphic_map));
	map = (dev->flags & KII_DF_CONSOLE) ? console_map[0] : graphic_map[0];
	index = 0;
	while ((index < sizeof(console_map)) && (map[index] != dev->id)) {

		index++;
	}
	focus   = index / KII_MAX_NR_CONSOLES;
	console = index % KII_MAX_NR_CONSOLES;
	if (! (KII_VALID_FOCUS_ID(focus) && KII_VALID_CONSOLE_ID(console) &&
		KII_VALID_DEVICE_ID(map[index]) && (map[index] == dev->id))) {

		KRN_DEBUG(1, "no %s device allowed (device-id %i)",
			(dev->flags & KII_DF_CONSOLE) ? "console" : "graphic",
			dev->id);
		dev->id = KII_INVALID_DEVICE;
		return -ENODEV;
	}
	if (kiidevice[dev->id]) {

		KRN_DEBUG(1, "device %i (%s %i-%i) is busy", dev->id,
			(dev->flags & KII_DF_CONSOLE) ? "console" : "graphic",
			focus, console);
		dev->id = KII_INVALID_DEVICE;
		return -EBUSY;
	}

	dev->focus_id = focus_map[dev->id];

	kiidevice[dev->id] = dev;
	return KII_EOK;
}

void kii_unregister_device(kii_device_t *dev)
{
	KRN_ASSERT(dev);
	KRN_ASSERT(KII_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(dev == kiidevice[dev->id]);
	KRN_ASSERT(!(dev->flags & KII_DF_FOCUSED));

	kiidevice[dev->id] = NULL;
	dev->focus_id = KII_INVALID_FOCUS;
	dev->id = KII_INVALID_DEVICE;
}


inline void kii_map_device(kii_u_t dev_id)
{
	kii_device_t *dev;
	kii_focus_t *f;

	if (! (KII_VALID_DEVICE_ID(dev_id) && (dev = kiidevice[dev_id]) &&
		KII_VALID_FOCUS_ID(focus_map[dev_id]) && 
		(f = kiifocus[focus_map[dev_id]]))) {

		KRN_DEBUG(2, "no target or focus for device %i, no map done",
			dev_id);
		return;
	}
	KRN_ASSERT(f->focus == NULL);
	KRN_DEBUG(2, "mapping device %i on focus %i", dev->id, f->id);

	f->focus = dev;

	f->ptr.x = dev->ptr.x;
	f->ptr.y = dev->ptr.y;
	f->ptr_min.x = dev->ptr_min.x;
	f->ptr_min.y = dev->ptr_min.y;
	f->ptr_max.x = dev->ptr_max.x;
	f->ptr_max.y = dev->ptr_max.y;

	dev->flags |= KII_DF_FOCUSED;

	if (dev->MapDevice) {

		(dev->MapDevice)(dev);
	}

	return;
}

/*	Do necessary actions to prepare mapping of device <dev>.
*/
inline kii_s_t kii_unmap_device(kii_u_t dev_id)
{
	kii_s_t err;
	kii_focus_t *f;
	kii_device_t *dev;

	if (! (KII_VALID_DEVICE_ID(dev_id) && kiidevice[dev_id] &&
		KII_VALID_FOCUS_ID(focus_map[dev_id]) &&
		(f = kiifocus[focus_map[dev_id]]))) {

		KRN_DEBUG(2, "no target or focus for device %i, no unmap done",
			dev_id);
		return -EINVAL;
	}

	if (! (dev = f->focus)) {

		return KGI_EOK;
	}

	KRN_DEBUG(2, "unmapping device %i from focus %i", dev->id, f->id);

	if (dev->UnmapDevice) {

		if ((err = dev->UnmapDevice(dev))) {

			return err;
		}
	}

	f->focus = NULL;

	dev->ptr.x     = f->ptr.x;
	dev->ptr.y     = f->ptr.y;
	dev->ptr_min.x = f->ptr_min.x;
	dev->ptr_min.y = f->ptr_min.y;
	dev->ptr_max.x = f->ptr_max.x;
	dev->ptr_max.y = f->ptr_max.y;

	dev->flags &= ~KII_DF_FOCUSED;

	return KGI_EOK;
}

kii_device_t *kii_current_focus(kii_u_t focus_id)
{
	KRN_ASSERT(KII_VALID_FOCUS_ID(focus_id));

	return kiifocus[focus_id] ? kiifocus[focus_id]->focus : NULL;
}

int kii_focus_of_task(void *task_ptr)
{
	struct task_struct *task = task_ptr;
	kii_focus_t *f;

	if (! task) {

		return -EINVAL;
	}

	/*	Try to determine the focus that delivers input to this task.
	**	In case there is no controlling tty for this process, try
	**	to take the parent's tty.
	*/
	while (!task->tty && (task->p_pptr || task->p_opptr)) {

		struct task_struct *next;
		next = task->p_pptr ? task->p_pptr : task->p_opptr;
		if (next == task) {

			break;
		}
		task = next;
	}
	/*	If even the parent had no controlling tty, we don't know
	**	what 'workplace group' this process belongs to, and cannot
	**	determine the focus to use. It's just a best effort...
	*/
	if (! task->tty) {

		return -EINVAL;
	}
	f = kiidev_focus(MINOR(task->tty->device) - 
		task->tty->driver.minor_start);
	return f ? f->id : -EINVAL;
}

kii_s_t kii_console_device(kii_s_t focus)
{
	kii_focus_t *f;
	int console;

	if (! KII_VALID_FOCUS_ID(focus)) {

		return -EINVAL;
	}
	f = kiifocus[focus];

	if (f && KII_VALID_CONSOLE_ID(f->curr_console)) {

		KRN_ASSERT(KII_VALID_CONSOLE_ID(f->curr_console));
		return f->console_map[f->curr_console];
	}


	for (console = 0; console < KII_MAX_NR_CONSOLES; console++) {

		if (KII_VALID_DEVICE_ID(console_map[focus][console])) {
			return console_map[focus][console];
		}
	}
	return -EINVAL;
}

/*
**	kernel input actions
*/

inline void kii_put_event(kii_focus_t *f, kii_event_t *event)
{
	KRN_ASSERT(f && event);

	if (f->focus && (f->focus->event_mask & (1 << event->any.type))) {

		(f->focus->HandleEvent)(f->focus, event);
	}
}
#ifdef	CONFIG_MAGIC_SYSRQ
#include <linux/sysrq.h>

static inline void do_sysrq(kii_focus_t *f, kii_event_t *event)
{
	if (event->any.type != KII_EV_KEY_PRESS) {

		return;
	}
	handle_sysrq(event->key.sym, f->pt_regs, NULL, 
		f->focus ? f->focus->tty : NULL);
	f->flags &= ~KII_FF_SYSTEM_REQUEST;
}
#endif

static inline void do_special(kii_focus_t *f, kii_event_t *event)
{
	if (event->any.type != KII_EV_KEY_PRESS) {

		return;
	}

	KRN_DEBUG(2, "doing special key %.4x", event->key.sym);

	switch(event->key.sym) {

	case K_VOID:
	case K_ENTER:
		return;

	case K_SH_REGS:
		if (f->pt_regs) {

			show_regs(f->pt_regs);
		}
		event->any.type = KII_EV_NOTHING;
		return;

	case K_SH_MEM:
		show_mem();
		event->any.type = KII_EV_NOTHING;
		return;

	case K_SH_STAT:
		show_state();
		event->any.type = KII_EV_NOTHING;
		return;

	case K_BREAK:
		return;

	case K_CONS:
		if (!KII_VALID_CONSOLE_ID(f->want_console) &&
			KII_VALID_CONSOLE_ID(f->last_console)) {

			f->want_console = f->last_console;
			f->flags |= KII_FF_PROCESS_BH;
			mark_bh(KEYBOARD_BH);
		}
		return;

	case K_CAPS:
		f->flags ^= KII_FF_CAPS_SHIFT;
		return;

	case K_NUM:
	case K_HOLD:
	case K_SCROLLFORW:
	case K_SCROLLBACK:
		return;

	case K_BOOT:
		ctrl_alt_del();
		event->any.type = KII_EV_NOTHING;
		return;

	case K_CAPSON:
		f->flags |= KII_FF_CAPS_SHIFT;
		return;

	case K_COMPOSE:
		f->dead = K_COMPOSE;
		return;

	case K_SAK:
		if (f->focus && f->focus->tty) {

			do_SAK((struct tty_struct *) f->focus->tty);
		}
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

			if (--search <  0) {

				search = KII_MAX_NR_CONSOLES - 1;
			}
		}

		if ((search != start) && 
			!KII_VALID_CONSOLE_ID(f->want_console)) {

			f->want_console = search;
			f->flags |= KII_FF_PROCESS_BH;
			mark_bh(KEYBOARD_BH);
		}
		return;
	}

	case K_INCRCONSOLE: {

		register kii_s_t start, search;

		if (! KII_VALID_CONSOLE_ID(f->curr_console)) {

			search = (start = 0) + 1;

		} else {

			KRN_ASSERT(f->curr_console < KII_MAX_NR_CONSOLES);
			search = ((start = f->curr_console) <
				KII_MAX_NR_DEVICES - 1) 
				? f->curr_console + 1 : 0;
		}

		while ((search != start) && !(
			(KII_VALID_DEVICE_ID(f->console_map[search]) &&
				kiidevice[f->console_map[search]]) ||
			(KII_VALID_DEVICE_ID(f->graphic_map[search]) &&
				kiidevice[f->graphic_map[search]]))) {

			if (++search >= KII_MAX_NR_CONSOLES) {

				search = 0;
			}
		}

		if ((search != start) &&
			!KII_VALID_CONSOLE_ID(f->want_console)) {

			f->want_console = search;
			f->flags |= KII_FF_PROCESS_BH;
			mark_bh(KEYBOARD_BH);
		}
		return;
	}

	case K_SPAWNCONSOLE:
		KRN_ASSERT(sizeof(pid_t) <= sizeof(kii_u_t));
		if (f->focus && ((pid_t) f->focus->spawnpid.priv_u)) {

			if (kill_proc(((pid_t) f->focus->spawnpid.priv_u), 
				f->focus->spawnsig.priv_u, 1)) {

				f->focus->spawnpid.priv_u = 0;
			}
		}
		event->any.type = KII_EV_NOTHING;
		return;

	case K_BARENUMLOCK:
		return;

	case K_TOGGLESCREEN:
		if (KII_VALID_DEVICE_ID(f->curr_console)) {

			f->want_console = f->curr_console;
			f->flags |= KII_FF_PROCESS_BH;
			mark_bh(KEYBOARD_BH);
		}
		return;

#ifdef	CONFIG_MAGIC_SYSRQ
	case K_SYSTEM_REQUEST:
		f->flags |= KII_FF_SYSTEM_REQUEST;
		return;
#endif

	default:
		KRN_DEBUG(0, "unknown special key %.4x", event->key.sym);
	}
}

static inline void do_modifier(kii_focus_t *f, kii_event_t *event)
{
	register kii_unicode_t ksym = event->key.sym;
	kii_u_t effect = f->effect;

	KRN_DEBUG(2, "old modifiers: %.2x %.2x %.2x %.2x, modifier key %.4x",
		f->effect, f->normal, f->locked, f->sticky, ksym);

	/*	The effect of STICKY and LOCKED modifiers is toggled whenever
	**	they are pressed.
	*/
	if ((K_FIRST_STICKY <= ksym) && (ksym < K_LAST_STICKY)) {

		KRN_DEBUG(2, "sticky modifer key");

		if (event->key.type == KII_EV_KEY_PRESS) {

			f->sticky ^= 1 << (ksym - K_FIRST_STICKY);	
		}
	}

	if ((K_FIRST_LOCKED <= ksym) && (ksym < K_LAST_LOCKED)) {

		KRN_DEBUG(2, "locked modifier key");

		if (event->key.type == KII_EV_KEY_PRESS) {

			f->locked ^= 1 << (ksym - K_FIRST_LOCKED);
		}
	}

	/*	If a NORMAL modifier is pressed at least once, it goes into
	**	effect and clears the effect of the corresponding LOCKED
	**	modifier. Thus there is no effect in the EFFECTIVE modifiers.
	**	The effect of the NORMAL modifier is not cleared until the
	**	last is released (if pressed multiple times), which clears
	**	the effect of the corresponding LOCKED modifier too.
	*/
	if ((K_FIRST_NORMAL <= ksym) && (ksym < K_LAST_NORMAL)) {

		register kii_u_t mask = 1 << (ksym - K_FIRST_NORMAL);

		KRN_DEBUG(2, "normal modifier key");

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

		npadch.key.type = KII_EV_KEY_PRESS;
		npadch.key.time = jiffies;
		npadch.key.code = 0xFFFF;
		npadch.key.sym  = f->npadch;
		npadch.key.effect = f->effect;
		npadch.key.normal = f->normal;
		npadch.key.locked = f->locked; 
		npadch.key.sticky = f->sticky;

		kii_put_event(f, &npadch);
	}

	KRN_DEBUG(2, "new modifers: %.2x %.2x %.2x %.2x", f->effect, 
		f->normal, f->locked, f->sticky);
}

static inline void do_ascii(kii_focus_t *f, kii_event_t *event)
{
	kii_s_t base = 10, value = event->key.sym - K_FIRST_ASCII;

	KRN_DEBUG(2, "doing ascii key %.4x", event->key.sym);

	if (value >= 10) {

		value -= 10;
		base = 16;
	}

	f->npadch = (f->npadch == K_VOID) ? value : (f->npadch*base + value);
	f->npadch %= (base == 16) ? 0x10000 : 100000;
}

static kii_unicode_t kii_dead_key[K_LAST_DEAD - K_FIRST_DEAD] = 
{
	0x0060,	/* ` grave	*/
	0x0027,	/* ' acute	*/
	0x005e,	/* ^ circumflex	*/
	0x007e,	/* ~ tilde	*/
	0x0022,	/* " diaeresis	*/
	0x002c	/* , cedilla	*/
};

static inline void do_dead(kii_focus_t *f, kii_event_t *event)
{
	kii_unicode_t dead = kii_dead_key[event->key.sym - K_FIRST_DEAD];

	KRN_DEBUG(2, "dead key %.4x", dead);
	f->dead = (f->dead == event->key.sym) ? K_VOID : dead;
}

static inline void do_action(kii_focus_t *f, kii_event_t *event)
{
	register kii_u_t sym = event->key.sym;

	if ((1 << event->any.type) & ~(KII_EM_KEY_PRESS | KII_EM_KEY_RELEASE)) {

		return;
	}

	KRN_DEBUG(2, "key %s, code %.2x, sym %.2x", 
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
		if (KII_VALID_CONSOLE_ID(sym) &&
			(event->key.type == KII_EV_KEY_PRESS)) {

			if (KII_VALID_CONSOLE_ID(f->want_console)) {

				if (f->want_console == sym) {

					f->flags |= KII_FF_PROCESS_BH;
					mark_bh(KEYBOARD_BH);
				} else {

					/* beep!!! */
				}

			} else {

				f->want_console = sym;
				f->flags |= KII_FF_PROCESS_BH;
				mark_bh(KEYBOARD_BH);
			}
		}
		return;

	case K_TYPE_SHIFT:
		if (sym < K_LAST_SHIFT) {

			do_modifier(f, event);
		}
		return;
	
	case K_TYPE_ASCII:
		if ((sym < K_LAST_ASCII) &&
			(event->key.type == KII_EV_KEY_PRESS)) {

			do_ascii(f, event);
		}
		return;

	case K_TYPE_DEAD:
		if ((sym < K_LAST_DEAD) &&
			(event->key.type == KII_EV_KEY_PRESS ||
			 event->key.type == KII_EV_KEY_REPEAT)) {

			do_dead(f, event);
		}
		return;
	}
}


void kii_handle_input(kii_event_t *event, void *regs)
{
	kii_focus_t *f;
	kii_u_t mask = 1 << event->any.type;
	KRN_ASSERT(event->any.focus < KII_MAX_NR_FOCUSES);

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

		if (f->flags & KII_FF_CAPS_SHIFT) {

			event->key.sym = keymap_toggled_case(event->key.sym);
		}
		KRN_DEBUG(2, "key %i %s, sym %.4x", event->key.code,
			(event->key.type == KII_EV_KEY_PRESS) ? "down" : "up",
			event->key.sym);
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

			case K_TYPE_FUNCTION:
			case K_TYPE_SPECIAL:
			case K_TYPE_NUMPAD:
			case K_TYPE_CONSOLE:
			case K_TYPE_CURSOR:
			case K_TYPE_SHIFT:
			case K_TYPE_META:
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

	f->pt_regs = regs;
	do_action(f, event);
	kii_put_event(f, event);
}

void kii_make_sound(kii_focus_t *f, kii_u_t frequency, kii_u_t duration)
{
	KRN_DEBUG(2, "kii_make_sound() not implemented yet!");
}

void kiidev_set_pointer_window(kii_device_t *dev,
	kii_s_t minx, kii_s_t maxx, kii_s_t miny, kii_s_t maxy)
{
	KRN_ASSERT(dev && KII_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(minx < maxx);
	KRN_ASSERT(miny < maxy);

	dev->ptr_min.x = minx;
	dev->ptr_min.y = miny;
	dev->ptr_max.x = maxx;
	dev->ptr_max.y = maxy;

	if (dev->ptr.x < minx)	dev->ptr.x = minx;
	if (dev->ptr.x >= maxx)	dev->ptr.x = maxx - 1;
	if (dev->ptr.y < miny)	dev->ptr.y = miny;
	if (dev->ptr.y >= maxy)	dev->ptr.y = maxy - 1;

	if (dev->flags & KII_DF_FOCUSED) {
	
		kii_focus_t *f = kiifocus[dev->focus_id];
		KRN_ASSERT(KII_VALID_FOCUS_ID(dev->focus_id) &&
			kiifocus[dev->focus_id]);

		f->ptr_min.x = minx;
		f->ptr_min.y = miny;
		f->ptr_max.x = maxx;
		f->ptr_max.y = maxy;
		f->ptr.x = dev->ptr.x;
		f->ptr.y = dev->ptr.y;
	}
}

const kii_ascii_t *kiidev_get_fnstring(kii_device_t *dev, kii_u_t key)
{
	KRN_ASSERT(dev && KII_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(KII_VALID_FOCUS_ID(dev->focus_id) && kiifocus[dev->focus_id]);

	return keymap_get_fnstring(&(kiifocus[dev->focus_id]->kmap), key);
}

kii_focus_t *kiidev_focus(kii_s_t dev_id)
{
	kii_u8_t *map;
	kii_s_t index;

	/*	If there is a (console) device registered for this ID, we
	**	take a shortcut to tell which focus serves this device.
	*/
	if (! KII_VALID_DEVICE_ID(dev_id)) {

		return NULL;
	}
	if (kiidevice[dev_id]) {

		KRN_ASSERT(kiidevice[dev_id]->id == dev_id);
		KRN_ASSERT(KII_VALID_FOCUS_ID(kiidevice[dev_id]->focus_id));
		return kiifocus[kiidevice[dev_id]->focus_id];
	}

	/*	Now we have to search which focus we belong to...
	*/
	map = console_map[0];
	index = sizeof(console_map) - 1;
	while ((0 <= index) && (map[index] != dev_id)) {

		index--;
	}
	return (index < 0) ? NULL : kiifocus[index / KII_MAX_NR_CONSOLES];
}


void kiidev_sync(kii_device_t *dev, kii_sync_flags_t what)
{
	KRN_DEBUG(2, "kiidev_sync() not implemented yet!");
}


/*	This routine runs with all interrupts enabled and does all the things
**	that may have to be done in response to an handled event, but may take
**	a reasonable long time.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
static 
#endif
void kii_bottomhalf(void)
{
	kii_u_t i;

	for (i = 0; i < KII_MAX_NR_FOCUSES; i++) {

		kii_s_t dev;
		kii_focus_t *f = kiifocus[i];

		if (! (f && (f->flags & KII_FF_PROCESS_BH) &&
			KII_VALID_CONSOLE_ID(f->want_console) &&
			(f->want_console != f->curr_console)) ||
			vt_dont_switch) {

			if (f) {
				f->want_console = KII_INVALID_CONSOLE;
			}
			continue;
		}
		dev = (f->focus_graph[f->want_console]
			? f->graphic_map : f->console_map)[f->want_console];
		if (! (KII_VALID_DEVICE_ID(dev) && kiidevice[dev])) {

			dev = (f->focus_graph[f->want_console]
				? f->console_map : f->graphic_map)
				[f->want_console];
		}
		if (! (KII_VALID_DEVICE_ID(dev) && kiidevice[dev])) {

			KRN_DEBUG(2, "invalid device %i (console %i)", 
				dev, f->want_console);
			f->want_console = KII_INVALID_CONSOLE;
			continue;
		}

		f->flags &= ~KII_FF_PROCESS_BH;

		switch (kii_unmap_device(dev)) {
		case -EINVAL:
			f->want_console = KII_INVALID_CONSOLE;
		case -EAGAIN:
			continue;
		case KGI_EOK:
			break;
		default:
			KRN_INTERNAL_ERROR;
		}

		switch (kgi_unmap_device(dev)) {
		case -EINVAL:
			f->want_console = KII_INVALID_CONSOLE;
		case -EAGAIN:
			kii_map_device(f->curr_console);
			continue;
		case KGI_EOK:
			break;
		default:
			KRN_INTERNAL_ERROR;
		}

		kgi_map_device(dev);
		kii_map_device(dev);

		f->last_console = f->curr_console;
		f->curr_console = f->want_console;
		f->want_console = KII_INVALID_CONSOLE;
	}
}

void kii_init(void)
{
	memset(kiifocus, 0, sizeof(kiifocus));
	memset(kiidevice, 0, sizeof(kiidevice));

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,2) /* 2.3.1 ??? */
	init_bh(KEYBOARD_BH, kii_bottomhalf);
#endif

	KRN_NOTICE("Kernel Input Interface " KII_REVISION);
}


#if 0
/*
**	public stuff
*/
void kgi_set_pointer(struct kgi_device *dev, ggi_uint x, ggi_uint y)
{
	dev->pointer.x = (x < dev->ptr_min.x) ? dev->ptr_min.x : 
		((x < dev->ptr_max.x) ? x : (dev->ptr_max.x - 1));

	dev->pointer.y = (y < dev->ptr_min.y) ? dev->ptr_min.y :
		((y < dev->ptr_max.y) ? y : (dev->ptr_max.y - 1));
}

#endif

/*
**	exported symbols
*/
#ifdef	EXPORT_SYMTAB
#include <linux/config.h>
#include <linux/module.h>

/*	KII manager interface
*/
EXPORT_SYMBOL(kii_handle_input);
EXPORT_SYMBOL(kii_register_device);
EXPORT_SYMBOL(kii_unregister_device);
EXPORT_SYMBOL(kii_register_input);
EXPORT_SYMBOL(kii_unregister_input);
EXPORT_SYMBOL(kii_map_device);
EXPORT_SYMBOL(kii_unmap_device);
EXPORT_SYMBOL(kii_current_focus);
EXPORT_SYMBOL(kiidev_focus);

#endif	/* #ifdef EXPORT_SYMTAB */

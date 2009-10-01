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
 * KII input manager
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

static kii_s_t initialized = 0;

/*
 * Input device administration.
 */
kii_focus_t  *kiifocus[KII_MAX_NR_FOCUSES];
kii_device_t *kiidevice[KII_MAX_NR_DEVICES];

static kii_u_t 
kii_next_input_id(kii_focus_t *f)
{
	kii_u_t input_id = f->next_input_id;
	kii_input_t *input;

	do {
		if (KII_MAX_NR_INPUTS <= ++input_id) 
			input_id = 0;

		input = f->inputs;
		while (input && (input->id != input_id)) 
			input = input->next;

		if (input == NULL) 
			return (input_id);	

	} while (input_id != f->next_input_id);

	return (KII_INVALID_INPUT);
}

/*
 * Register an input device to a focus. We keep KII_IC_2DPOINTER devices
 * first in the linked list to tell easily if a has a pointer.
 * return values:
 * 	KII_EOK successful
 * 	KII_EINVAL parameters invalid
 */
kii_s_t
kii_register_input(kii_u_t focus, kii_input_t *dev, int reset)
{
	kii_focus_t *f;
	kii_input_t *prev;
	kii_u_t input_id;

	/*
	 * Auto-assign to the next focus that needs one of these.
	 */
	if (dev && (focus == KII_INVALID_FOCUS)) {
		focus = 0;
		while (KII_VALID_FOCUS_ID(focus)) {
			if (kiifocus[focus] == NULL) 
				break;

			prev = (kiifocus[focus])->inputs;
			while (prev != NULL) {
				if ((dev->events & prev->events & KII_EM_POINTER) ||
					(dev->events & prev->events & KII_EM_KEY)) {
						prev = NULL;  
						break; 
				}
				if (prev->next == NULL) 
					break;

				prev = prev->next;
			}
			if (prev != NULL)
				break;

			focus++;
		} 
	}

	if (!(dev && KII_VALID_FOCUS_ID(focus))) {
		KRN_ERROR("Invalid parameters: focus = %i, dev = %p",
			focus, dev);
		return (KII_EINVAL);
	}

	f = kiifocus[focus];
	if (!f || reset) {
 		kii_s_t i;

		KRN_DEBUG(2, "Allocating focus %i...", focus);
		f = kiifocus[focus] = kgi_kmalloc(sizeof(kii_focus_t));
		if (!f) { 
			KRN_ERROR("Focus %i allocation failed.", focus);
			return (KII_ENOMEM);
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
			KRN_DEBUG(3, "Found valid KII graphic map %d", i);
			f->curr_console = i;
			f->focus = kiidevice[f->graphic_map[i]];
		}

		if ((KII_VALID_DEVICE_ID(f->console_map[i]) &&
			kiidevice[f->console_map[i]])) {
			KRN_DEBUG(3, "Found valid KII console map %d", i);
			f->curr_console = i;
			f->focus = kiidevice[f->console_map[i]];
		}

		if (f->focus) {
			f->focus->flags |= KII_DF_FOCUSED;
			if (f->focus->MapDevice) {
				(f->focus->MapDevice)(f->focus);
			}
		}

		KRN_DEBUG(2, "Focus %i allocated, focused on console %i",
			focus, f->curr_console);
	}
	
	input_id = kii_next_input_id(f);
	if (input_id == KII_INVALID_INPUT) {
		KRN_ERROR("Could not get valid input ID.");
		return (KII_EBUSY);
	}
	dev->id = input_id;
	dev->focus = f->id;

	if ((prev = f->inputs)) {
		while (prev->next && (prev->next->events & KII_EM_POINTER)) 
			prev = prev->next;

		dev->next  = prev->next;
		prev->next = dev;
	} else {
		dev->next  = NULL;
		f->inputs  = dev;
	}

	f->flags |= (dev->events & KII_EM_POINTER) ? KII_FF_HAS_POINTER : 0;
	return (KII_EOK);
}

/*
 * Unregister a previously registered input device from its focus.
 * If dev is not valid or dev was not registered, we do nothing.
 */
void 
kii_unregister_input(kii_input_t *dev)
{
	kii_input_t *prev;
	kii_focus_t *f; 

	if (! (dev && KII_VALID_FOCUS_ID(dev->focus) && 
		(f = kiifocus[dev->focus]))) {
		KRN_ERROR("Invalid paramters: dev = %p, focus = %i",
			dev, dev ? dev->focus : KII_INVALID_FOCUS);
		return;
	}
	KRN_ASSERT(dev->focus == f->id);

	if (dev != f->inputs) {
		prev = f->inputs;
		while (prev && (prev->next != dev)) 
			prev = prev->next;

		if (prev) {
			prev->next = dev->next;
			dev->next  = NULL;
			dev->focus = KII_INVALID_FOCUS;
			dev->id    = KII_INVALID_INPUT;

		} else {
			/*
			 * The device is not listed in that focus. 
			 * We better report...
			 */
			KRN_ERROR("Device %p ('%s %s') not in list?",
				dev, dev->vendor, dev->model);
			return;
		}

	} else {	/* if (dev != f->inputs) ... */
		f->inputs  = dev->next;
		dev->next  = NULL;
		dev->focus = KII_INVALID_FOCUS;
		dev->id    = KII_INVALID_INPUT;

		/*
		 * Even if there are no input devices left, just to keep
		 * the focus hanging around. However, free the keymap
		 * and reset it to the default one.
		 */
		if (!f->inputs) 
			keymap_reset(&f->kmap);
	}

	f->flags &= ~KII_FF_HAS_POINTER;
	f->flags |= (f->inputs && (KII_EM_POINTER & f->inputs->events)) 
			? KII_FF_HAS_POINTER : 0;
}

/*
 * Clone a console device e.g. copy the source to the target device
 * then register.
 */
kii_device_t *
kii_clone_console(kii_u_t source, kii_u_t target)
{
	kii_device_t *src = NULL, *dest = NULL;
	kii_s_t error;

	if (!(KII_VALID_CONSOLE_ID(source) && KII_VALID_CONSOLE_ID(target)))
		return (NULL);

	if (!((src = kiidevice[source]) && (src->flags & KII_DF_CONSOLE)))
		return (NULL);

	if (target == source)
		return (src);

	if (!(dest = kgi_kmalloc(sizeof(kii_device_t))))
		return (NULL);

	*dest = *src;

	dest->id = KII_INVALID_DEVICE;
	dest->focus_id = KII_INVALID_FOCUS;
	dest->flags &= ~KII_DF_FOCUSED;
	dest->flags |= KII_DF_CLONED;

	error = kii_register_device(dest, target);

	if (error) {
		KRN_DEBUG(2, "Registration of cloned KII console failed.");
		kgi_kfree(dest);
		return (NULL);
	}

	return (dest);
}

/* 
 * Delete a clone previously created by kii_clone_console().
 */
kii_s_t 
kii_delete_clone(kii_u_t dev_id)
{
	kii_device_t *dev;

	if (!KII_VALID_CONSOLE_ID(dev_id))
		return (KII_EINVAL);

	if (!((dev = kiidevice[dev_id]) && (dev->flags & KII_DF_CLONED)))
		return (KII_EINVAL);

	kii_unregister_device(dev);

	kgi_kfree(dev);

	return (KII_EOK);
}

kii_s_t 
kii_register_device(kii_device_t *dev, kii_u_t index)
{
	kii_u_t focus, console;
	kii_u8_t *map;

	KRN_ASSERT(dev);
	if (!(dev && KII_VALID_CONSOLE_ID(index))) {
		KRN_ERROR("Invalid arguments %p, %i", 
			dev, index);
		return (KII_EINVAL);
	}
	dev->id = (dev->flags & KII_DF_CONSOLE) 
		? index : index + KII_MAX_NR_CONSOLES;

	KRN_ASSERT(sizeof(console_map) == sizeof(graphic_map));
	map = (dev->flags & KII_DF_CONSOLE) ? console_map[0] : graphic_map[0];
	index = 0;
	while ((index < sizeof(console_map)) && (map[index] != dev->id)) 
		index++;

	focus   = index / KII_MAX_NR_CONSOLES;
	console = index % KII_MAX_NR_CONSOLES;
	if (!(KII_VALID_FOCUS_ID(focus) && KII_VALID_CONSOLE_ID(console) &&
		KII_VALID_DEVICE_ID(map[index]) && (map[index] == dev->id))) {
		KRN_ERROR("No %s device allowed (device-id %i)",
			(dev->flags & KII_DF_CONSOLE) ? "console" : "graphic",
			dev->id);
		dev->id = KII_INVALID_DEVICE;
		return (KII_ENODEV);
	}
	if (kiidevice[dev->id]) {
		KRN_ERROR("Device %i (%s %i-%i) is busy", dev->id,
			(dev->flags & KII_DF_CONSOLE) ? "console" : "graphic",
			focus, console);
		dev->id = KII_INVALID_DEVICE;
		return (KII_EBUSY);
	}

	dev->focus_id = focus_map[dev->id];

	kiidevice[dev->id] = dev;

	KRN_DEBUG(2, "KII device %i registered.", dev->id);

	return (KII_EOK);
}

void 
kii_unregister_device(kii_device_t *dev)
{
	KRN_ASSERT(dev);
	KRN_ASSERT(KII_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(dev == kiidevice[dev->id]);
	KRN_ASSERT(!(dev->flags & KII_DF_FOCUSED));

	KRN_DEBUG(2, "KII device %i unregistered.", dev->id);

	kiidevice[dev->id] = NULL;
	dev->focus_id = KII_INVALID_FOCUS;
	dev->id = KII_INVALID_DEVICE;
}

void 
kii_map_device(kii_u_t dev_id)
{
	kii_device_t *dev;
	kii_focus_t *f;

	if (!(KII_VALID_DEVICE_ID(dev_id) && (dev = kiidevice[dev_id]) &&
		KII_VALID_FOCUS_ID(focus_map[dev_id]) && 
		(f = kiifocus[focus_map[dev_id]]))) {
		KRN_ERROR("No target or focus for device %i, no map done.", dev_id);
		return;
	}
	KRN_ASSERT(f->focus == NULL);
	KRN_DEBUG(3, "Mapping device %i on focus %i", dev->id, f->id);

	f->focus = dev;
	f->curr_console = dev_id % KII_MAX_NR_CONSOLES;

	f->ptr.x = dev->ptr.x;
	f->ptr.y = dev->ptr.y;
	f->ptr_min.x = dev->ptr_min.x;
	f->ptr_min.y = dev->ptr_min.y;
	f->ptr_max.x = dev->ptr_max.x;
	f->ptr_max.y = dev->ptr_max.y;

	dev->flags |= KII_DF_FOCUSED;

	if (dev->MapDevice) 
		(dev->MapDevice)(dev);

	return;
}

/*
 * Do necessary actions to prepare mapping of device <dev>.
 */
kii_s_t 
kii_unmap_device(kii_u_t dev_id)
{
	kii_s_t err;
	kii_focus_t *f;
	kii_device_t *dev;

	if (!(KII_VALID_DEVICE_ID(dev_id) && kiidevice[dev_id] &&
		KII_VALID_FOCUS_ID(focus_map[dev_id]) &&
		(f = kiifocus[focus_map[dev_id]]))) {
		KRN_ERROR("No target or focus for device %i, no unmap done.", dev_id);
		return (KII_EINVAL);
	}

	if (!(dev = f->focus))
		return (KII_EOK);

	KRN_DEBUG(3, "Unmapping device %i from focus %i", dev->id, f->id);

	if (dev->UnmapDevice) {
		if ((err = dev->UnmapDevice(dev))) {
			return (err);
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

	return (KII_EOK);
}

kii_device_t *
kii_current_focus(kii_u_t focus_id)
{

	KRN_ASSERT(KII_VALID_FOCUS_ID(focus_id));

	return (kiifocus[focus_id] ? kiifocus[focus_id]->focus : NULL);
}

void 
kii_put_event(kii_focus_t *f, kii_event_t *event)
{

	KRN_ASSERT(f && event);

	if (f->focus && (f->focus->event_mask & (1 << event->any.type))) 
		(f->focus->HandleEvent)(f->focus, event);
}

/*
 * Poll the device e.g. all the inputs of its focus if any.
 */
void 
kii_poll_device(kii_u_t dev_id, kii_event_t *event)
{
	kii_focus_t *f;
	kii_device_t *dev;
	kii_input_t *input;
	int scancode = -1;

	/* Raz the event for no event. */
	bzero(event, sizeof(event));

	if (!(KII_VALID_DEVICE_ID(dev_id) && kiidevice[dev_id] &&
		KII_VALID_FOCUS_ID(focus_map[dev_id]) &&
		(f = kiifocus[focus_map[dev_id]]))) {
		return;
	}
	if (!(dev = f->focus)) 
		return;

	/* Poll the focus inputs. Stop at first that has data. */
	for (input = f->inputs; input; input = input->next) {
		if (input->Poll) {
			scancode = input->Poll(input);
			if (scancode != -1)
				break;
		}
	}

	if (scancode == -1)
		return;

	/* Set dontdispatch to avoid re-entrance into KII code. */
	event->any.dontdispatch = 1;

	if (input->Parse)
		input->Parse(input, event, scancode);
}

#if 0
int 
kii_focus_of_task(void *task_ptr)
{
	struct task_struct *task = task_ptr;
	kii_focus_t *f;

	if (! task) {

		return KII_EINVAL;
	}

	/*
	 * Try to determine the focus that delivers input to this task.
	 * In case there is no controlling tty for this process, try
	 * to take the parent's tty.
	 */
	while (!task->tty && (task->parent || task->real_parent)) {

		struct task_struct *next;
		next = task->parent ? task->parent : task->real_parent;
		if (next == task) {

			break;
		}
		task = next;
	}

	/*
	 * If even the parent had no controlling tty, we don't know
	 * what 'workplace group' this process belongs to, and cannot
	 * determine the focus to use. It's just a best effort...
	 */
	if (! task->tty) {

		return KII_EINVAL;
	}
	f = kiidev_focus(
		minor(task->tty->device) -
		task->tty->driver.minor_start);

	return f ? f->id : KII_EINVAL;
}
#endif

kii_s_t 
kii_console_device(kii_s_t focus)
{
	kii_focus_t *f;
	int console;

	if (!KII_VALID_FOCUS_ID(focus))
		return (KII_EINVAL);

	f = kiifocus[focus];

	if (f && KII_VALID_CONSOLE_ID(f->curr_console)) {
		KRN_ASSERT(KII_VALID_CONSOLE_ID(f->curr_console));
		return (f->console_map[f->curr_console]);
	}


	for (console = 0; console < KII_MAX_NR_CONSOLES; console++) {
		if (KII_VALID_DEVICE_ID(console_map[focus][console])) {
			return (console_map[focus][console]);
		}
	}
	return (KII_EINVAL);
}

void 
kiidev_set_pointer_window(kii_device_t *dev, kii_s_t minx, kii_s_t maxx,
	  kii_s_t miny, kii_s_t maxy)
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

void 
kiidev_sync(kii_device_t *dev, kii_sync_flags_t what)
{
	
	KRN_DEBUG(4, "kiidev_sync() not implemented yet!");
}

const 
kii_ascii_t *kiidev_get_fnstring(kii_device_t *dev, kii_u_t key)
{
	
	KRN_ASSERT(dev && KII_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(KII_VALID_FOCUS_ID(dev->focus_id) && kiifocus[dev->focus_id]);

	return (keymap_get_fnstring(&(kiifocus[dev->focus_id]->kmap), key));
}

kii_focus_t *
kiidev_focus(kii_s_t dev_id)
{
	kii_u8_t *map;
	kii_s_t index;

	/*
	 * If there is a (console) device registered for this ID, we
	 * take a shortcut to tell which focus serves this device.
	 */
	if (!KII_VALID_DEVICE_ID(dev_id)) 
		return (NULL);

	if (kiidevice[dev_id]) {
		KRN_ASSERT(kiidevice[dev_id]->id == dev_id);
		KRN_ASSERT(KII_VALID_FOCUS_ID(kiidevice[dev_id]->focus_id));
		return (kiifocus[kiidevice[dev_id]->focus_id]);
	}

	/* Now we have to search which focus we belong to... */
	map = console_map[0];
	index = sizeof(console_map) - 1;
	while ((0 <= index) && (map[index] != dev_id)) 
		index--;

	return ((index < 0) ? NULL : kiifocus[index / KII_MAX_NR_CONSOLES]);
}

/*
 * Backdoor for the keyboard drivers.
 */
int
kii_configure(int flags)
{

	if (!initialized) {
		memset(kiifocus, 0, sizeof(kiifocus));
		memset(kiidevice, 0, sizeof(kiidevice));	
		initialized = 1;
	}

	return (KII_EOK);
}


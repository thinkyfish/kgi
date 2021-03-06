/*-
 * Copyright (c) 1995-2000 Steffen Seeger
 * Copyright (c) 2002-2004 Nicholas Souchu
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
 * KGI display manager.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_MUTEX
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>

SYSINIT(kgi, SI_SUB_COPYRIGHT, SI_ORDER_FIRST, kgi_init, NULL);

static int initialized = 0;

kgi_mutex_t kgi_lock;

kgi_u8_t console_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
kgi_u8_t graphic_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
kgi_u8_t focus_map[CONFIG_KGII_MAX_NR_DEVICES];
kgi_u8_t display_map[CONFIG_KGII_MAX_NR_DEVICES];

MALLOC_DEFINE(M_KGI, "kgi", "KGI data structures");

static kgi_device_t 	*kgidevice[KGI_MAX_NR_DEVICES];
static kgi_display_t 	*kgidpy[KGI_MAX_NR_DISPLAYS];

/*
 * Display handling.
 */
kgi_error_t
kgidev_display_command(kgi_device_t *dev, kgi_u_t cmd, void *data)
{

	if ((dev->flags & KGI_DF_FOCUSED) == 0) {
		KGI_ERROR("Protocol error.");
		return (KGI_EPROTO);
	}
	if (KGI_VALID_DISPLAY_ID(dev->dpy_id) == 0 ||
	    (kgidpy[dev->dpy_id]) == 0) {
		KGI_DEBUG(1, "No valid display device.");
		return (KGI_ENODEV);
	}
	KGI_ASSERT((cmd & KGIC_TYPE_MASK) == KGIC_DISPLAY_COMMAND);
	return ((kgidpy[dev->dpy_id]->Command == NULL) ? -ENXIO :
	    (kgidpy[dev->dpy_id]->Command)(kgidpy[dev->dpy_id], cmd, data));
}

/*
 * Check if a mode is valid. This returns a valid <mode> and KGI_EOK if
 * the mode can be done. NOTE: <cmd> must be either KGI_TC_PROPOSE or
 * KGI_TC_CHECK. <dpy> and <mode> must be valid.
 */
static kgi_error_t
kgidpy_check_mode(kgi_display_t *dpy, kgi_mode_t *m, kgi_timing_command_t cmd)
{
	kgi_error_t err;

	KGI_ASSERT(dpy);
	KGI_ASSERT(m);
	KGI_ASSERT(cmd == KGI_TC_PROPOSE || cmd == KGI_TC_CHECK);

	if (m->dev_mode == NULL && dpy->mode_size) {
		m->dev_mode = kgi_kmalloc(dpy->mode_size);
		if (m->dev_mode == NULL) {
			KGI_ERROR("Out of memory.");
			return (KGI_ENOMEM);
		}
		m->flags |= KGI_MF_ALLOCATED;
	}

	err = (dpy->CheckMode)(dpy, cmd, m->img, m->images, m->dev_mode,
		m->resource, __KGI_MAX_NR_RESOURCES);

	if (err != KGI_EOK) {
		if (m->flags & KGI_MF_ALLOCATED) {
			kgi_kfree(m->dev_mode);
			m->dev_mode = NULL;
		}
		return (err);
	}
#if 0
	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {
		if (m->resource[i] &&
			(m->resource[i]->type == KGI_RT_ACCELERATOR)) {
			kgi_accel_t *accel = (kgi_accel_t *) m->resource[i];
			kgi_queue_head_t *idle;
			if ((idle = kgi_kmalloc(sizeof(*idle))) == NULL) {
				KGI_ERROR("out of memory!");
				m->resource[i] = NULL;
			}
			accel->idle = idle;
			kgi_init_queue_head(accel->idle);
		}
	}
#endif
	return (KGI_EOK);
}

static void
kgidpy_release_mode(kgi_display_t *dpy, kgi_mode_t *m)
{

#ifdef notyet
	kgi_u_t i;

	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {
		if (m->resource[i] &&
			(m->resource[i]->type == KGI_RT_ACCELERATOR)) {
			kgi_accel_t *accel = (kgi_accel_t *) m->resource[i];
			kgi_kfree(accel->idle);
			accel->idle = NULL;
		}
	}
#endif

	if (m->dev_mode) {
		if (m->flags & KGI_MF_ALLOCATED) {
			kgi_kfree(m->dev_mode);
			m->flags &= ~KGI_MF_ALLOCATED;
			m->dev_mode = NULL;
		} else {
			KGI_ERROR("Loosing dev_mode allocated statically.");
			m->dev_mode = NULL;
		}
	}
}

static void
kgidpy_set_mode(kgi_display_t *dpy, kgi_mode_t *m)
{

	KGI_ASSERT(dpy);
	KGI_ASSERT(m);
	KGI_ASSERT(dpy->mode_size ? m->dev_mode != NULL : 1);

	(dpy->SetMode)(dpy, m->img, m->images, m->dev_mode);
}

static void
kgidpy_unset_mode(kgi_display_t *dpy, kgi_mode_t *m)
{

	KGI_ASSERT(dpy);
	KGI_ASSERT(m);
	KGI_ASSERT(dpy->mode_size ? m->dev_mode != NULL : 1);

	if (dpy->UnsetMode)
		(dpy->UnsetMode)(dpy, m->img, m->images, m->dev_mode);
}

/*
 * Device handling.
 */
kgi_error_t
kgi_register_device(kgi_device_t *dev, kgi_u_t index)
{
	kgi_s_t err;
	kgi_u_t focus, console;
	kgi_u8_t *map;

	if (dev == NULL && KGI_VALID_CONSOLE_ID(index) == 0) {
		KGI_ERROR("Invalid arguments %p, %i", dev, index);
		return (KGI_EINVAL);
	}
	dev->id = (dev->flags & KGI_DF_CONSOLE) ? index :
		index + KGI_MAX_NR_CONSOLES;

	KGI_ASSERT(sizeof(console_map) == sizeof(graphic_map));
	map = (dev->flags & KGI_DF_CONSOLE) ? console_map[0] : graphic_map[0];
	index = 0;
	while ((index < sizeof(console_map)) && (map[index] != dev->id))
		index++;

	focus   = index / KGI_MAX_NR_CONSOLES;
	console = index % KGI_MAX_NR_CONSOLES;
	if ((KGI_VALID_FOCUS_ID(focus) == 0 && KGI_VALID_CONSOLE_ID(console) &&
	    KGI_VALID_DEVICE_ID(map[index]) && (map[index] == dev->id))) {
		KGI_ERROR("No %s device allowed (dev %i)",
		(dev->flags & KGI_DF_CONSOLE) ? "console" : "graphic", dev->id);
		dev->id = KGI_INVALID_DEVICE;
		return (KGI_ENODEV);
	}

	if (kgidevice[dev->id]) {
		KGI_DEBUG(1, "Device %i (%s %i-%i) is busy", dev->id,
			(dev->flags & KGI_DF_CONSOLE) ? "console" : "graphic",
			focus, console);
		dev->id = KGI_INVALID_DEVICE;
		return (KGI_EBUSY);
	}

	dev->dpy_id = display_map[dev->id];
	if (KGI_VALID_DISPLAY_ID(dev->dpy_id) == 0 && kgidpy[dev->dpy_id]) {
		KGI_ERROR("No display to attach device (dpy %i, dev %i)",
			dev->dpy_id, dev->id);
		dev->dpy_id = KGI_INVALID_DISPLAY;
		dev->id = KGI_INVALID_DEVICE;
		return (KGI_ENODEV);
	}

	err = kgidpy_check_mode(kgidpy[dev->dpy_id], dev->mode, KGI_TC_PROPOSE);
	if (err != KGI_EOK) {
		KGI_ERROR("Initial mode check with dpy %i failed (%i)",
			dev->dpy_id, err);
		dev->dpy_id = KGI_INVALID_DISPLAY;
		dev->id = KGI_INVALID_DEVICE;
		return (KGI_EINVAL);
	}

	if ((dev->flags & KGI_DF_CONSOLE) == 0) {
		kgi_display_t *dpy = kgidpy[dev->dpy_id];

		while (dpy) {
			dpy->graphic++;
			(dpy->IncRefcount)(dpy);
			dpy = dpy->prev;
		}
	}
	kgidevice[dev->id] = dev;

	KGI_DEBUG(2, "KGI device %i registered.", dev->id);

	return (KGI_EOK);
}

void
kgi_unregister_device(kgi_device_t *dev)
{

	KGI_ASSERT(dev);
	KGI_ASSERT(KGI_VALID_DEVICE_ID(dev->id));
	KGI_ASSERT(dev == kgidevice[dev->id]);
	KGI_ASSERT((dev->flags & KGI_DF_FOCUSED) == 0);

	if ((dev->flags & KGI_DF_CONSOLE) == 0) {
		kgi_display_t *dpy = kgidpy[dev->dpy_id];

		while (dpy) {
			(dpy->DecRefcount)(dpy);
			dpy->graphic--;
			dpy = dpy->prev;
		}
	}
	kgidevice[dev->id] = NULL;

	kgidpy_release_mode(kgidpy[dev->dpy_id], dev->mode);

	KGI_DEBUG(2, "KGI device %i unregistered.", dev->id);

	dev->dpy_id = KGI_INVALID_DISPLAY;
	dev->id = KGI_INVALID_DEVICE;
}

/*
 * Checking of non CONSOLE devices is not allowed. All attached devices must
 * be consoles.
 */
static kgi_error_t
kgi_check_device_mode(kgi_display_t *dpy, kgi_s_t id)
{
	kgi_error_t error;
	kgi_device_t *dev;
	kgi_mode_t mode;
	kgi_u_t i;

	KGI_ASSERT(KGI_VALID_DEVICE_ID(id));

	dev = kgidevice[id];
	if (dev == NULL)
		return (KGI_EOK);

	/* Refuse to reattach a device not in console mode. */
	error = KGI_ENODEV;
	if (dev->flags & KGI_DF_CONSOLE) {
		bzero(&mode, sizeof(mode));

		mode.flags = dev->mode->flags & ~KGI_MF_ALLOCATED;
		mode.images = dev->mode->images;

		mode.img[0].size.x = dev->mode->img[0].size.x;
		mode.img[0].size.y = dev->mode->img[0].size.y;
		mode.img[0].virt.x = dev->mode->img[0].virt.x;
		mode.img[0].virt.y = dev->mode->img[0].virt.y;
		mode.img[0].cam = dev->mode->img[0].cam;
		mode.img[0].fam = dev->mode->img[0].fam;

		for (i = 0 ; i < __KGI_MAX_NR_ATTRIBUTES; i++)
			mode.img[0].bpca[i] = dev->mode->img[0].bpca[i];
		for (i = 0 ; i < __KGI_MAX_NR_ATTRIBUTES; i++)
			mode.img[0].bpfa[i] = dev->mode->img[0].bpfa[i];

		error = kgidpy_check_mode(dpy, &mode, KGI_TC_PROPOSE);

		kgidpy_release_mode(dpy, &mode);
	}

	return (error);
}

/*
 * Shall only be called once the device mode has been checked. The previous
 * display mode is freed here.
 */
static void
kgi_reattach_device(kgi_display_t *dpy, kgi_s_t id)
{
	kgi_event_t event;
	kgi_error_t error;
	kgi_device_t *dev;
	int was_focused;

	KGI_ASSERT(KGI_VALID_DEVICE_ID(id));

	dev = kgidevice[id];
	if (dev == NULL)
		return;

	/* Refuse to reattach a device not in console mode. */
	was_focused = 0;
	if (dev->flags & KGI_DF_CONSOLE) {
		KGI_DEBUG(2, "Reattaching device %i", id);

		/* Unmap any focused device. */
		if (dev->flags & KGI_DF_FOCUSED) {
			was_focused = 1;
			kgi_unmap_device(id);
		}

		/* Free the previous display mode. */
		kgidpy_release_mode(dpy, dev->mode);

		/* Allocate the new display mode, supposed to be ok. */
		error = kgidpy_check_mode(dpy, dev->mode, KGI_TC_PROPOSE);
		if (error != KGI_EOK) {
			KGI_ERROR("Failed to reattach device %i (%d)",
				  id, error);
		}
		/* Notify the device of the display change. */
		event.notice.command = KGI_EVENT_NOTICE_NEW_DISPLAY;

		/* Send the event before remapping the device. */
		if (kgidevice[id]->HandleEvent)
			kgidevice[id]->HandleEvent(kgidevice[id], &event);

		if (was_focused)
			kgi_map_device(id);
	}
}

static inline kgi_u_t
kgi_can_do_console(kgi_display_t *dpy)
{
	kgi_mode_t mode;
	kgi_error_t err;

	memset(&mode, 0, sizeof(mode));
	mode.images = 1;
	mode.img[0].flags |= KGI_IF_TEXT16;

	err = kgidpy_check_mode(dpy, &mode, KGI_TC_PROPOSE);

	if (mode.dev_mode && (mode.flags & KGI_MF_ALLOCATED)) {
		kgi_kfree(mode.dev_mode);
		mode.dev_mode = NULL;
	}

	return ((err == KGI_EOK) ? (mode.img[0].flags & KGI_IF_TEXT16) : 0);
}

static inline kgi_u_t
kgi_must_do_console(kgi_display_t *dpy)
{
	kgi_u_t i;

	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {
		if ((display_map[i] == dpy->id) && kgidevice[i] &&
		    (kgidevice[i]->flags & KGI_DF_TEXT16)) {
			return (1);
		}
	}
	return (0);
}

/*
 * Register <dpy> under <id>. If <id> is negative, we search for the first
 * free one.
 */
kgi_error_t
kgi_register_display(kgi_display_t *dpy, kgi_u_t id)
{
	kgi_display_t *prev;
	kgi_error_t error;
	kgi_u_t i;

	KGI_DEBUG(2, "Registering %s %s display with id %i",
		dpy->vendor, dpy->model, id);

	if (KGI_VALID_DISPLAY_ID(id) == 0) {
		for (id = 0; (id < KGI_MAX_NR_DISPLAYS) && kgidpy[id]; id++)
			; /* Nothing. */
		if (id > KGI_MAX_NR_DISPLAYS)
			return (KGI_ENOMEM);

		KGI_DEBUG(2, "Auto-assigned new id %i", id);
	}

	if (KGI_VALID_DISPLAY_ID(id) == 0 || dpy == NULL ||
	    (kgidpy[id] && kgidpy[id]->graphic)) {
		KGI_ERROR("Can't replace display %i", id);
		return (KGI_EINVAL);
	}

	if (kgi_must_do_console(dpy) && kgi_can_do_console(dpy) == 0) {
		KGI_DEBUG(1, "New display has too but can't do console.");
		kgidpy[id] = dpy->prev;
		dpy->prev = NULL;
		dpy->id = KGI_INVALID_DISPLAY;
		return (KGI_EINVAL);
	}

	error = KGI_EOK;
	/* Check mode for each device registered. */
	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {
		if (display_map[i] == id) {
			/* If can't set device mode, fail. */
			error = kgi_check_device_mode(dpy, i);
			if (error)
				return (KGI_EINVAL);
		}
	}

	dpy->id = id;
	dpy->graphic = 0;
	dpy->prev = kgidpy[id];
	kgidpy[dpy->id] = dpy;

	/*
	 * If no error during checks, check display specific modes and
	 * set the mode for the focused device if any.
	 */
	if (error == 0) {
		for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {
			if (display_map[i] == id) {
				kgi_reattach_device(dpy, i);
			}
		}
		KGI_NOTICE("Display %i: %s %s registered.", dpy->id,
			   dpy->vendor, dpy->model);
	}

	prev = dpy->prev;
	while (prev) {
		(prev->IncRefcount)(prev);
		prev = prev->prev;
	}

	return (error);
}

void
kgi_unregister_display(kgi_display_t *dpy)
{
	kgi_display_t *prev;
	kgi_u_t i;

	KGI_ASSERT(dpy);
	KGI_ASSERT(KGI_VALID_DISPLAY_ID(dpy->id));
	KGI_ASSERT(kgidpy[dpy->id] == dpy);
	KGI_ASSERT(dpy->graphic == 0);

	kgidpy[dpy->id] = dpy->prev;

	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {
		if (display_map[i] == dpy->id) {
			kgi_reattach_device(dpy->prev, i);
		}
	}

	prev = kgidpy[dpy->id];
	while (prev) {
		(prev->DecRefcount)(prev);
		prev = prev->prev;
	}

	KGI_NOTICE("Display %i: %s %s unregistered.", dpy->id, dpy->vendor,
		dpy->model);
	dpy->id = KGI_INVALID_DISPLAY;
	dpy->prev = NULL;
}

kgi_error_t
kgi_unmap_device(kgi_u_t dev_id)
{
	kgi_device_t *dev;
	kgi_display_t *dpy;
	kgi_error_t err;

	dpy = kgidpy[display_map[dev_id]];
	if ((KGI_VALID_DEVICE_ID(dev_id) && kgidevice[dev_id] &&
	    KGI_VALID_DISPLAY_ID(display_map[dev_id]) && (dpy)) == 0) {
		KGI_DEBUG(3, "No target or display to unmap.");
		return (KGI_EINVAL);
	}

	if (dpy->focus == NULL)
		return (KGI_EOK);

	dev = dpy->focus;
	KGI_DEBUG(3, "Unmapping device %i from display %i", dev->id, dpy->id);

	if (dev->UnmapDevice) {
		err = dev->UnmapDevice(dev);
		if (err)
			return (err);
	}

	kgidpy_unset_mode(dpy, dev->mode);

	dpy->focus = NULL;
	dev->flags &= ~KGI_DF_FOCUSED;

	return (KGI_EOK);
}

void
kgi_map_device(kgi_u_t dev_id)
{
	kgi_device_t *dev;
	kgi_display_t *dpy;

	if ((KGI_VALID_DEVICE_ID(dev_id) &&
	    KGI_VALID_DISPLAY_ID(display_map[dev_id])) == 0) {
		KGI_DEBUG(3, "No target or display for device %i, no map done.",
			  dev_id);
		return;
	}

	dev = kgidevice[dev_id];
	dpy = kgidpy[display_map[dev_id]];

	KGI_ASSERT(dpy->focus == NULL);

	KGI_DEBUG(3, "Mapping device %i on display %i", dev->id, dpy->id);

	dpy->focus = dev;
	dev->flags |= KGI_DF_FOCUSED;

	kgidpy_set_mode(dpy, dev->mode);

	if (dev->MapDevice)
		(dev->MapDevice)(dev);
}

kgi_device_t *
kgi_current_focus(kgi_u_t dpy_id)
{

	KGI_ASSERT(KGI_VALID_DISPLAY_ID(dpy_id));
	KGI_ASSERT(kgidpy[dpy_id]);

	return kgidpy[dpy_id]->focus;
}

kgi_u_t
kgi_current_devid(kgi_u_t dpy_id)
{

	if (kgidpy[dpy_id] == 0)
		return (0);

	if (kgidpy[dpy_id]->focus == 0)
		return (-1);

	return (kgidpy[dpy_id]->focus->id);
}

kgi_error_t
kgi_display_registered(kgi_u_t dpy_id)
{

	return ((KGI_VALID_DISPLAY_ID(dpy_id) && kgidpy[dpy_id]) ? KGI_EOK :
		 KGI_ENODEV);
}

/*
 * KGI manager initialization.
 *
 * All kernel services need to be accessible when kgi_init is called,
 * especially PCI-scanning, kgi_kmalloc*() services. We first
 * initialize global variables and then get the services running.
 */
static void
kgi_init_maps(kgi_u_t nr_displays, kgi_u_t nr_focuses)
{
	kgi_u_t console, device, display, focus;

	display = 0;
	for (device = 0; device < CONFIG_KGII_MAX_NR_CONSOLES; device++) {
		focus = device / (CONFIG_KGII_MAX_NR_CONSOLES /
				  CONFIG_KGII_MAX_NR_FOCUSES);
		console = device % (CONFIG_KGII_MAX_NR_CONSOLES /
				    CONFIG_KGII_MAX_NR_FOCUSES);
		if ((KGI_VALID_FOCUS_ID(focus) &&
		    KGI_VALID_CONSOLE_ID(console)) == 0)
			continue;

		KGI_DEBUG(4, "Mapping device %i on focus %i, display %i, "
			  "console %i", device, focus, display, console);

		console_map[focus][console] = device;
		graphic_map[focus][console] = device +
			CONFIG_KGII_MAX_NR_CONSOLES;

		focus_map[device] =
		    focus_map[device + CONFIG_KGII_MAX_NR_CONSOLES] = focus;

		display_map[device] =
		    display_map[device + CONFIG_KGII_MAX_NR_CONSOLES] = display;

		if ((device % CONFIG_KGII_MAX_NR_FOCUSES) ==
		    CONFIG_KGII_MAX_NR_FOCUSES - 1) {
			if (nr_displays > nr_focuses) {
				display++;
				nr_displays--;
			}
		}
		if (console == (CONFIG_KGII_MAX_NR_CONSOLES /
		    CONFIG_KGII_MAX_NR_FOCUSES) - 1) {
			nr_focuses--;
			nr_displays--;
			display++;
		}
	}
}

kgi_u_t
kgi_attr_bits(const kgi_u8_t *bpa)
{
	kgi_u_t bits;

	bits = 0;
	if (bpa) {
		while (*bpa) {
			bits += *(bpa++);
		}
	}

	return (bits);
}

/*
 * Initialize the KGI system if not already performed.
 */
void
kgi_init(void)
{
	kgi_u_t nr_displays, nr_focuses;

	KGI_DEBUG(3, "Initializing KGI.");
	kii_configure(0);

	if (!initialized) {
		memset(display_map, 0xFF, sizeof(display_map));
		memset(focus_map,   0xFF, sizeof(focus_map));
		memset(console_map, 0xFF, sizeof(console_map));
		memset(graphic_map, 0xFF, sizeof(graphic_map));

		memset(kgidpy, 0, sizeof(kgi_display_t));
		memset(kgidevice, 0, sizeof(kgi_device_t));

		nr_displays = 0;

		nr_displays += dpy_null_init(nr_displays, CONFIG_KGI_DISPLAYS);
		KGI_DEBUG(1, "%i displays initialized.", nr_displays);

		nr_focuses = 0;
		/* XXX	nr_focuses  = focus_init(); */

		kgi_init_maps(nr_displays, nr_focuses);

		kgi_mutex_init(&kgi_lock, "KGI Giant lock.");

		initialized = 1;
		KGI_DEBUG(3, "KGI Initialized.");
	}

	return;
}

/* ----------------------------------------------------------------------------
**	KGI display manager
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**	Copyright (C)	2002-2004	Nicholas Souchu
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
int console_initialized = 0;

kgi_mutex_t kgi_lock;

kgi_u8_t console_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
kgi_u8_t graphic_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
kgi_u8_t focus_map[CONFIG_KGII_MAX_NR_DEVICES];
kgi_u8_t display_map[CONFIG_KGII_MAX_NR_DEVICES];

MALLOC_DEFINE(M_KGI, "kgi", "KGI data structures");

static kgi_device_t	*kgidevice[KGI_MAX_NR_DEVICES];
static kgi_display_t	*kgidpy[KGI_MAX_NR_DISPLAYS];

/*
**	display handling
*/
kgi_error_t kgidev_display_command(kgi_device_t *dev, kgi_u_t cmd, void *data)
{
	if (! (dev->flags & KGI_DF_FOCUSED)) {

		KRN_ERROR("protocol error");
		return KGI_EPROTO;
	}
	if (! KGI_VALID_DISPLAY_ID(dev->dpy_id) ||
		(NULL == kgidpy[dev->dpy_id])) {

		KRN_DEBUG(1, "no valid display device");
		return KGI_ENODEV;
	}
	KRN_ASSERT((cmd & KGIC_TYPE_MASK) == KGIC_DISPLAY_COMMAND);
	return (NULL == kgidpy[dev->dpy_id]->Command)
		? -ENXIO
		: (kgidpy[dev->dpy_id]->Command)(kgidpy[dev->dpy_id], cmd, data);
}

/*	Check if a mode is valid. This returns a valid <mode> and KGI_EOK if
**	the mode can be done. NOTE: <cmd> must be either KGI_TC_PROPOSE or
**	KGI_TC_CHECK. <dpy> and <mode> must be valid.
*/
static kgi_error_t kgidpy_check_mode(kgi_display_t *dpy, kgi_mode_t *m,
	kgi_timing_command_t cmd)
{
	kgi_error_t err;

	KRN_ASSERT(dpy);
	KRN_ASSERT(m);
	KRN_ASSERT(cmd == KGI_TC_PROPOSE || cmd == KGI_TC_CHECK);

	if (!m->dev_mode && dpy->mode_size) {

		m->dev_mode = kgi_kmalloc(dpy->mode_size);
		if (! m->dev_mode) {
			
			KRN_ERROR("out of memory");
			return KGI_ENOMEM;
		}
		m->flags |= KGI_MF_ALLOCATED;
	}

	err = (dpy->CheckMode)(dpy, cmd,
		m->img, m->images, m->dev_mode,
		m->resource, __KGI_MAX_NR_RESOURCES);

	if (err) {
		if (m->flags & KGI_MF_ALLOCATED) {
			kgi_kfree(m->dev_mode);
			m->dev_mode = NULL;
		}
		return err;
	}

#if 0
	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {

		if (m->resource[i] && 
			(m->resource[i]->type == KGI_RT_ACCELERATOR)) {

			kgi_accel_t *accel = (kgi_accel_t *) m->resource[i];
			kgi_queue_head_t *idle;
			if (NULL == (idle = kgi_kmalloc(sizeof(*idle)))) {

				KRN_ERROR("out of memory!");
				m->resource[i] = NULL;
			}
			accel->idle = idle;
			kgi_init_queue_head(accel->idle);
		}
	}
#endif

	return KGI_EOK;
}

static void kgidpy_release_mode(kgi_display_t *dpy, kgi_mode_t *m)
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
			KRN_ERROR("Loosing dev_mode allocated statically");
			m->dev_mode = NULL;
		}
	}
}
	

static void kgidpy_set_mode(kgi_display_t *dpy, kgi_mode_t *m)
{
	KRN_ASSERT(dpy);
	KRN_ASSERT(m);
	KRN_ASSERT(dpy->mode_size ? m->dev_mode != NULL : 1);

	(dpy->SetMode)(dpy, m->img, m->images, m->dev_mode);
}

static void kgidpy_unset_mode(kgi_display_t *dpy, kgi_mode_t *m)
{
	KRN_ASSERT(dpy);
	KRN_ASSERT(m);
	KRN_ASSERT(dpy->mode_size ? m->dev_mode != NULL : 1);

	if (dpy->UnsetMode) {

		(dpy->UnsetMode)(dpy, m->img, m->images, m->dev_mode);
	}
}


/*
**	device handling
*/
kgi_error_t kgi_register_device(kgi_device_t *dev, kgi_u_t index)
{
	kgi_s_t err;
	kgi_u_t focus, console;
	kgi_u8_t *map;

	if (! (dev && KGI_VALID_CONSOLE_ID(index))) {

		KRN_ERROR("invalid arguments %p, %i", dev, index);
		return KGI_EINVAL;
	}
	dev->id = (dev->flags & KGI_DF_CONSOLE) 
		? index : index + KGI_MAX_NR_CONSOLES;

	KRN_ASSERT(sizeof(console_map) == sizeof(graphic_map));
	map = (dev->flags & KGI_DF_CONSOLE) ? console_map[0] : graphic_map[0];
	index = 0;
	while ((index < sizeof(console_map)) && (map[index] != dev->id)) {

		index++;
	}
	focus   = index / KGI_MAX_NR_CONSOLES;
	console = index % KGI_MAX_NR_CONSOLES;
	if (! (KGI_VALID_FOCUS_ID(focus) && KGI_VALID_CONSOLE_ID(console) &&
		KGI_VALID_DEVICE_ID(map[index]) && (map[index] == dev->id))) {

			KRN_ERROR("no %s device allowed (dev %i)",
			(dev->flags & KGI_DF_CONSOLE) ? "console" : "graphic",
			dev->id);
		dev->id = KGI_INVALID_DEVICE;
		return KGI_ENODEV;
	}
	if (kgidevice[dev->id]) {

		KRN_DEBUG(1, "device %i (%s %i-%i) is busy", dev->id,
			(dev->flags & KGI_DF_CONSOLE) ? "console" : "graphic",
			focus, console);
		dev->id = KGI_INVALID_DEVICE;
		return KGI_EBUSY;
	}

	dev->dpy_id = display_map[dev->id];
	if (! (KGI_VALID_DISPLAY_ID(dev->dpy_id) && kgidpy[dev->dpy_id])) {

		KRN_ERROR("no display to attach device (dpy %i, dev %i)",
			dev->dpy_id, dev->id);
		dev->dpy_id = KGI_INVALID_DISPLAY;
		dev->id  = KGI_INVALID_DEVICE;
		return KGI_ENODEV;
	}
	if ((err = kgidpy_check_mode(kgidpy[dev->dpy_id], dev->mode,
		KGI_TC_PROPOSE))) {

		KRN_ERROR("initial mode check with dpy %i failed (%i)",
			dev->dpy_id, err);
		dev->dpy_id = KGI_INVALID_DISPLAY;
		dev->id = KGI_INVALID_DEVICE;
		return KGI_EINVAL;
	}
	if (! (dev->flags & KGI_DF_CONSOLE)) {

		kgi_display_t *dpy = kgidpy[dev->dpy_id];

		while (dpy) {

			dpy->graphic++;
			(dpy->IncRefcount)(dpy);
			dpy = dpy->prev;
		}
	}
	kgidevice[dev->id] = dev;

	KRN_DEBUG(2, "KGI device %i registered", dev->id);

	return KGI_EOK;
}

void kgi_unregister_device(kgi_device_t *dev)
{
	KRN_ASSERT(dev);
	KRN_ASSERT(KGI_VALID_DEVICE_ID(dev->id));
	KRN_ASSERT(dev == kgidevice[dev->id]);
	KRN_ASSERT(! (dev->flags & KGI_DF_FOCUSED));

	if (! (dev->flags & KGI_DF_CONSOLE)) {

		kgi_display_t *dpy = kgidpy[dev->dpy_id];

		while (dpy) {

			(dpy->DecRefcount)(dpy);
			dpy->graphic--;
			dpy = dpy->prev;
		}
	}
	kgidevice[dev->id] = NULL;

	kgidpy_release_mode(kgidpy[dev->dpy_id], dev->mode);

	KRN_DEBUG(2, "KGI device %i unregistered", dev->id);

	dev->dpy_id = KGI_INVALID_DISPLAY;
	dev->id = KGI_INVALID_DEVICE;
}

/* Checking of non CONSOLE devices is not allowed. All attached devices must
 * be consoles.
 */
static kgi_error_t kgi_check_device_mode(kgi_display_t *dpy, kgi_s_t id)
{
	kgi_error_t error = KGI_ENODEV;
	kgi_device_t *dev = kgidevice[id];
	kgi_mode_t mode;
	kgi_u_t i;

	KRN_ASSERT(KGI_VALID_DEVICE_ID(id));

	if (!dev)
		return KGI_EOK;

	/* Refuse to reattach a device not in console mode.
	 */
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

	return error;		
}

/* Shall only be call once the device mode has been checked. The previous
 * display mode is freed here.
 */
static void kgi_reattach_device(kgi_display_t *dpy, kgi_s_t id)
{
	kgi_event_t event;
	kgi_error_t error;
	kgi_device_t *dev = kgidevice[id];
	int was_focused = 0;

	KRN_ASSERT(KGI_VALID_DEVICE_ID(id));

	if (!dev)
		return;

	/* Refuse to reattach a device not in console mode.
	 */
	if (dev->flags & KGI_DF_CONSOLE) {

		KRN_DEBUG(2, "reattaching device %i", id);

		/* Unmap any focused device */
		if (dev->flags & KGI_DF_FOCUSED) {
			was_focused = 1;
			kgi_unmap_device(id);
		}

		/* Free the previous display mode */
		kgidpy_release_mode(dpy, dev->mode);

		/* Allocate the new display mode, supposed to be ok */
		if ((error = kgidpy_check_mode(dpy, dev->mode, KGI_TC_PROPOSE)))
			KRN_ERROR("Failed to reattach device %i (%d)", id, error);
		
		/* Notify the device of the display change */
		event.notice.command = KGI_EVENT_NOTICE_NEW_DISPLAY;
		
		/* Send the event before remapping the device */
		if (kgidevice[id]->HandleEvent) {
			kgidevice[id]->HandleEvent(kgidevice[id], &event);
		}

		if (was_focused)
			kgi_map_device(id);
	}
}

static inline kgi_u_t kgi_can_do_console(kgi_display_t *dpy)
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

	return (KGI_EOK == err) ? (mode.img[0].flags & KGI_IF_TEXT16) : 0;
}

static inline kgi_u_t kgi_must_do_console(kgi_display_t *dpy)
{
	kgi_u_t i;

	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {

		if ((display_map[i] == dpy->id) && kgidevice[i] &&
			(kgidevice[i]->flags & KGI_DF_TEXT16)) {

			return 1;
		}
	}
	return 0;
}

/*	Register <dpy> under <id>. If <id> is negative, we search for the first
**	free one.
*/
kgi_error_t kgi_register_display(kgi_display_t *dpy, kgi_u_t id)
{
	kgi_display_t *prev;
	kgi_error_t error = KGI_EOK;
	kgi_u_t i;

	KRN_DEBUG(2, "registering %s %s display with id %i", 
		dpy->vendor, dpy->model, id);

	if (! KGI_VALID_DISPLAY_ID(id)) {

		for (id = 0; (id < KGI_MAX_NR_DISPLAYS) && kgidpy[id]; id++) {
		}
		if (KGI_MAX_NR_DISPLAYS <= id) {

			return KGI_ENOMEM;
		}
		KRN_DEBUG(2, "auto-assigned new id %i", id);
	}

	if (!KGI_VALID_DISPLAY_ID(id) || !dpy ||
		(kgidpy[id] && kgidpy[id]->graphic)) {

		KRN_ERROR("can't replace display %i", id);
		return KGI_EINVAL;
	}

	if (kgi_must_do_console(dpy) && !kgi_can_do_console(dpy)) {

		KRN_DEBUG(1, "new display has to but can't do console");
		kgidpy[id] = dpy->prev;
		dpy->prev = NULL;
		dpy->id = KGI_INVALID_DISPLAY;
		return KGI_EINVAL;
	}

	/* Check mode for each device registered */
	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {

		if (display_map[i] == id) {

			/* If can't set device mode, fail */
			if ((error = kgi_check_device_mode(dpy, i)))
				return KGI_EINVAL;
		}
	}

	dpy->id = id;
	dpy->graphic = 0;
	dpy->prev = kgidpy[id];
	kgidpy[dpy->id] = dpy;

	/* If no error during checks, check display specific modes and
	 * set the mode for the focused device if any.
	 */
	if (!error) {
		for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {
			if (display_map[i] == id) {
				kgi_reattach_device(dpy, i);
			}
		}
		KRN_NOTICE("display %i: %s %s registered", dpy->id,
			   dpy->vendor, dpy->model);
	}

	prev = dpy->prev;
	while (prev) {

		(prev->IncRefcount)(prev);
		prev = prev->prev;
	}

	return error;
}

void kgi_unregister_display(kgi_display_t *dpy)
{ 
	kgi_display_t *prev;
	kgi_u_t i;

	KRN_ASSERT(dpy);
	KRN_ASSERT(KGI_VALID_DISPLAY_ID(dpy->id));
	KRN_ASSERT(kgidpy[dpy->id] == dpy);
	KRN_ASSERT(dpy->graphic == 0);

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

	KRN_NOTICE("display %i: %s %s unregistered", dpy->id,
		dpy->vendor, dpy->model);
	dpy->id   = KGI_INVALID_DISPLAY;
	dpy->prev = NULL;
}
	
kgi_error_t kgi_unmap_device(kgi_u_t dev_id)
{
	kgi_device_t *dev;
	kgi_display_t *dpy;

	if (! (KGI_VALID_DEVICE_ID(dev_id) && kgidevice[dev_id] &&
		KGI_VALID_DISPLAY_ID(display_map[dev_id]) &&
		(dpy = kgidpy[display_map[dev_id]]))) {

		KRN_DEBUG(3, "no target or display, no unmap done");
		return KGI_EINVAL;
	}

	if (!(dev = dpy->focus))
		return KGI_EOK;

	KRN_DEBUG(3, "unmapping device %i from display %i", dev->id, dpy->id);

	if (dev->UnmapDevice) {

		kgi_error_t err;
		if ((err = dev->UnmapDevice(dev))) {

			return err;
		}
	}

	kgidpy_unset_mode(dpy, dev->mode);

	dpy->focus = NULL;
	dev->flags &= ~KGI_DF_FOCUSED;

	return KGI_EOK;
}

void kgi_map_device(kgi_u_t dev_id)
{
	kgi_device_t *dev;
	kgi_display_t *dpy;

	if (! (KGI_VALID_DEVICE_ID(dev_id) && (dev = kgidevice[dev_id]) &&
		KGI_VALID_DISPLAY_ID(display_map[dev_id]) &&
		(dpy = kgidpy[display_map[dev_id]]))) {

		KRN_DEBUG(3, "no target or display for device %i, no map done",
			dev_id);
		return;
	}
	KRN_ASSERT(dpy->focus == NULL);

	KRN_DEBUG(3, "mapping device %i on display %i", dev->id, dpy->id);

	dpy->focus = dev;
	dev->flags |= KGI_DF_FOCUSED;

	kgidpy_set_mode(dpy, dev->mode);

	if (dev->MapDevice) {

		(dev->MapDevice)(dev);
	}
}

kgi_device_t *kgi_current_focus(kgi_u_t dpy_id)
{
	KRN_ASSERT(KGI_VALID_DISPLAY_ID(dpy_id));
	KRN_ASSERT(kgidpy[dpy_id]);

	return kgidpy[dpy_id]->focus;
}

kgi_u_t kgi_current_devid(kgi_u_t dpy_id)
{
	if (!kgidpy[dpy_id])
		return 0;

	if (!kgidpy[dpy_id]->focus)
		return -1;

	return (kgidpy[dpy_id]->focus->id);
}

kgi_error_t kgi_display_registered(kgi_u_t dpy_id)
{
	return (KGI_VALID_DISPLAY_ID(dpy_id) && kgidpy[dpy_id]) 
		? KGI_EOK : KGI_ENODEV;
}

/*
**	KGI manager initialization
*/

/*	All kernel services need to be accessible when kgi_init is called,
**	especially PCI-scanning, kgi_malloc_*() services. We first
**	initialize global variables and then get the services running.
*/
static void kgi_init_maps(kgi_u_t nr_displays, kgi_u_t nr_focuses)
{
	kgi_u_t display = 0, device;
	for (device = 0; device < CONFIG_KGII_MAX_NR_CONSOLES; device++) {

		kgi_u_t focus   = device / (CONFIG_KGII_MAX_NR_CONSOLES /
						CONFIG_KGII_MAX_NR_FOCUSES);
		kgi_u_t console = device % (CONFIG_KGII_MAX_NR_CONSOLES /
						CONFIG_KGII_MAX_NR_FOCUSES);
		if (! (KGI_VALID_FOCUS_ID(focus) &&
			KGI_VALID_CONSOLE_ID(console))) {

			continue;
		}

		KRN_DEBUG(4, "mapping device %i on focus %i, display %i, console %i",
			device, focus, display, console);

		console_map[focus][console] = device;
		graphic_map[focus][console] = device + 
			CONFIG_KGII_MAX_NR_CONSOLES;

		focus_map[device] = 
		focus_map[device + CONFIG_KGII_MAX_NR_CONSOLES] = focus;

		display_map[device] = 
		display_map[device + CONFIG_KGII_MAX_NR_CONSOLES] = display;

		if ((device % CONFIG_KGII_MAX_NR_FOCUSES) ==
			CONFIG_KGII_MAX_NR_FOCUSES-1) {

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

kgi_u_t kgi_attr_bits(const kgi_u8_t *bpa)
{
	kgi_u_t bits = 0;
	
	if (bpa) {

		while (*bpa) {

			bits += *(bpa++);
		}
	}

	return bits;
}

/*
 * Initialize the KGI system if not already performed.
 */
void kgi_init(void)
{
	kgi_u_t nr_displays, nr_focuses;

	kii_configure(0);

	if (!initialized) {
		
		memset(display_map, 0xFF, sizeof(display_map));
		memset(focus_map,   0xFF, sizeof(focus_map));
		memset(console_map, 0xFF, sizeof(console_map));
		memset(graphic_map, 0xFF, sizeof(graphic_map));
		
		memset(kgidpy, 0, sizeof(kgidpy));
		memset(kgidevice, 0, sizeof(kgidevice));

		nr_displays = 0;
		
		nr_displays += dpy_null_init(nr_displays, CONFIG_KGI_DISPLAYS);
		KRN_DEBUG(1, "%i displays initialized", nr_displays);
		
		nr_focuses = 0;
		/* XXX	nr_focuses  = focus_init(); */
		
		kgi_init_maps(nr_displays, nr_focuses);

		kgi_mutex_init(&kgi_lock, "KGI Giant lock");

		initialized = 1;
	}

	return;
}

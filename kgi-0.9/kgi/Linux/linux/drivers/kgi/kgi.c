/* ----------------------------------------------------------------------------
**	KGI display manager
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
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
**	$Log: kgi.c,v $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	VERSION		"$Revision: 1.19 $"

#define	DEBUG_LEVEL	1

#include <linux/version.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/malloc.h>

#if LINUX_VERSION_CODE < 0x020300
typedef struct wait_queue *wait_queue_head_t;
#define	init_waitqueue_head(head)	*((wait_queue_head_t*)(head)) = NULL
#define	set_current_state(a)		current->state = (a)
#endif

#define	KGI_SYS_NEED_IO
#include <linux/kgii.h>

/*
#include "default/clut.inc"
#include "default/pointer.inc"
*/

kgi_u8_t console_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
kgi_u8_t graphic_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
kgi_u8_t focus_map[CONFIG_KGII_MAX_NR_DEVICES];
kgi_u8_t display_map[CONFIG_KGII_MAX_NR_DEVICES];


static kgi_device_t	*kgidevice[KGI_MAX_NR_DEVICES];
static kgi_display_t	*kgidpy[KGI_MAX_NR_DISPLAYS];

/*
**	display handling
*/
#if 0
/*	Hide the cursor and pointer, even if they are in hardware.
*/
void kgidev_hide_gadgets(kgi_device_t *dev)
{
	kgi_display_t *dpy = kgidpy[dev->dpy_id];

	KRN_ASSERT(dev->flags & KGI_DF_FOCUSED);

	if ((dev->flags & KGI_DF_POINTER_SHOWN) && dpy->PointerHide) {

		(dpy->PointerHide)(dpy);
		dev->flags &= ~(KGI_DF_POINTER_FB_MODIFIED |
			KGI_DF_POINTER_SHOWN);
	}

	if ((dev->flags & KGI_DF_CURSOR_SHOWN) && dpy->CursorHide) {

		(dpy->CursorHide)(dpy);
		dev->flags &= ~(KGI_DF_CURSOR_FB_MODIFIED |
			KGI_DF_CURSOR_SHOWN);
	}
}

/*	Undo fb modifications, does nothing if we have hardware cursor/pointer.
*/
void kgidev_undo_gadgets(kgi_device_t *dev)
{
	kgi_display_t *dpy = kgidpy[dev->dpy_id];

	KRN_ASSERT(dev->flags & KGI_DF_FOCUSED);

	if ((dev->flags & KGI_DF_POINTER_FB_MODIFIED) && dpy->PointerUndo) {

		(dpy->PointerUndo)(dpy);
		dev->flags &= ~(KGI_DF_POINTER_FB_MODIFIED |
			KGI_DF_POINTER_SHOWN);
	}

	if ((dev->flags & KGI_DF_CURSOR_FB_MODIFIED) && dpy->CursorUndo) {

		(dpy->CursorUndo)(dpy);
		dev->flags &= ~(KGI_DF_CURSOR_FB_MODIFIED |
			KGI_DF_CURSOR_SHOWN);
	}
}

/*	Update cursor and pointer, undo fb modifications before.
*/
void kgidev_show_gadgets(kgi_device_t *dev)
{
	kgi_display_t *dpy = kgidpy[dev->dpy_id];

	KRN_ASSERT(dev->flags & KGI_DF_FOCUSED);

	if ((dev->flags & KGI_DF_POINTER_FB_MODIFIED) && dpy->PointerUndo) {

		(dpy->PointerUndo)(dpy);
	}

	if ((dev->flags & KGI_DF_CURSOR_FB_MODIFIED) && dpy->CursorUndo) {

		(dpy->CursorUndo)(dpy);
	}

	if (dev->flags & KGI_DF_SHOW_GADGETS) {

		if ((dev->flags & KGI_DF_CURSOR_TO_SHOW) && dpy->CursorShow) {

			(dpy->CursorShow)(dpy, dev->cur.x, dev->cur.y);
			dev->flags |= KGI_DF_CURSOR_SHOWN | (dpy->CursorUndo
				? KGI_DF_CURSOR_FB_MODIFIED : 0);
		}

		if ((dev->flags & KGI_DF_POINTER_TO_SHOW) && dpy->PointerShow) {

			(dpy->PointerShow)(dpy, dev->ptr.x,dev->ptr.y);
			dev->flags |= KGI_DF_POINTER_SHOWN | (dpy->PointerUndo
				? KGI_DF_POINTER_FB_MODIFIED : 0);
		}
	}
}
#endif

kgi_error_t kgidev_display_command(kgi_device_t *dev, kgi_u_t cmd, void *in,
	void **out, kgi_size_t *out_size)
{
	if (! (dev->flags & KGI_DF_FOCUSED)) {

		return -EPROTO;
	}
	if (!KGI_VALID_DISPLAY_ID(dev->dpy_id) ||
		(NULL == kgidpy[dev->dpy_id])) {

		KRN_DEBUG(1, "no valid display device");
		return -ENODEV;
	}
	KRN_ASSERT((cmd & KGIC_TYPE_MASK) == KGIC_DISPLAY_COMMAND);
	return (NULL == kgidpy[dev->dpy_id]->Command)
		? -ENXIO
		: (kgidpy[dev->dpy_id]->Command)(kgidpy[dev->dpy_id],
			cmd, in, out, out_size);
}

kgi_error_t kgidev_mode_command(kgi_device_t *dev, kgi_u_t cmd, void *in,
	void **out, kgi_size_t *out_size)
{
	kgic_mode_context_t ctx;
	KRN_ASSERT((cmd & KGIC_TYPE_MASK) == KGIC_MODE_COMMAND);

	if (! (KGI_VALID_DISPLAY_ID(dev->dpy_id) && 
		kgidpy[dev->dpy_id] &&
		kgidpy[dev->dpy_id]->ModeCommand)) {

		KRN_DEBUG(1, "invalid mode request");
		return -EINVAL;
	}
	ctx.dpy      = kgidpy[dev->dpy_id];
	ctx.dev_mode = dev->mode->dev_mode;
	ctx.images   = dev->mode->images;
	ctx.img      = dev->mode->img;
	return (NULL == ctx.dpy->ModeCommand)
		? -ENXIO
		: (ctx.dpy->ModeCommand)(&ctx, cmd, in, out, out_size);
}


/*	Check if a mode is valid. This returns a valid <mode> and KGI_EOK if
**	the mode can be done. NOTE: <cmd> must be either KGI_TC_PROPOSE or
**	KGI_TC_CHECK. <dpy> and <mode> must be valid.
*/
inline kgi_error_t kgidpy_check_mode(kgi_display_t *dpy, kgi_mode_t *m,
	kgi_timing_command_t cmd)
{
	kgi_error_t err;
	kgi_u_t	i;

	KRN_ASSERT(dpy);
	KRN_ASSERT(m);
	KRN_ASSERT(cmd == KGI_TC_PROPOSE || cmd == KGI_TC_CHECK);

	if (!m->dev_mode && dpy->mode_size) {

		m->dev_mode = kmalloc(dpy->mode_size, GFP_KERNEL);
		if (! m->dev_mode) {

			return -ENOMEM;
		}
	}

	err = (dpy->CheckMode)(dpy, cmd,
		m->img, m->images, m->dev_mode,
		m->resource, __KGI_MAX_NR_RESOURCES);

	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {

		if (m->resource[i] && 
			(m->resource[i]->type == KGI_RT_ACCELERATOR)) {

			kgi_accel_t *accel = (kgi_accel_t *) m->resource[i];
			wait_queue_head_t *idle;
			if (NULL == (idle = kmalloc(sizeof(*idle),
				GFP_KERNEL))) {

				KRN_DEBUG(1, "out of memory!");
				m->resource[i] = NULL;
			}
			accel->idle = idle;
			init_waitqueue_head(accel->idle);
		}
	}

	if (err) {

		kfree(m->dev_mode);
		m->dev_mode = NULL;
		return err;
	}
	return KGI_EOK;
}

static inline void kgidpy_release_mode(kgi_display_t *dpy, kgi_mode_t *m)
{
	kgi_u_t i;

	for (i = 0; i < __KGI_MAX_NR_RESOURCES; i++) {

		if (m->resource[i] && 
			(m->resource[i]->type == KGI_RT_ACCELERATOR)) {

			kgi_accel_t *accel = (kgi_accel_t *) m->resource[i];
			kfree(accel->idle);
			accel->idle = NULL;
		}
	}

	if (m->dev_mode) {

		kfree(m->dev_mode);
		m->dev_mode = NULL;
	}
}
	

static inline void kgidpy_set_mode(kgi_display_t *dpy, kgi_mode_t *m)
{
	KRN_ASSERT(dpy);
	KRN_ASSERT(m);
	KRN_ASSERT(dpy->mode_size ? m->dev_mode != NULL : 1);

	(dpy->SetMode)(dpy, m->img, m->images, m->dev_mode);
}

static inline void kgidpy_unset_mode(kgi_display_t *dpy, kgi_mode_t *m)
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

	KRN_ASSERT(dev);
	if (! (dev && KGI_VALID_CONSOLE_ID(index))) {

		KRN_DEBUG(1, "invalid arguments %p, %i", dev, index);
		return -EINVAL;
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

		KRN_DEBUG(1, "no %s device allowed (device-id %i)",
			(dev->flags & KGI_DF_CONSOLE) ? "console" : "graphic",
			dev->id);
		dev->id = KGI_INVALID_DEVICE;
		return -ENODEV;
	}
	if (kgidevice[dev->id]) {

		KRN_DEBUG(1, "device %i (%s %i-%i) is busy", dev->id,
			(dev->flags & KGI_DF_CONSOLE) ? "console" : "graphic",
			focus, console);
		dev->id = KGI_INVALID_DEVICE;
		return -EBUSY;
	}

	dev->dpy_id = display_map[dev->id];
	if (! (KGI_VALID_DISPLAY_ID(dev->dpy_id) && kgidpy[dev->dpy_id])) {

		KRN_DEBUG(1, "no display to attach device (dpy %i, dev %i)",
			dev->dpy_id, dev->id);
		dev->dpy_id = KGI_INVALID_DISPLAY;
		dev->id  = KGI_INVALID_DEVICE;
		return -ENODEV;
	}
	if ((err = kgidpy_check_mode(kgidpy[dev->dpy_id], dev->mode,
		KGI_TC_PROPOSE))) {

		KRN_DEBUG(1, "initial mode check with dpy %i failed (err = %i)",
			dev->dpy_id, err);
		dev->dpy_id = KGI_INVALID_DISPLAY;
		dev->id = KGI_INVALID_DEVICE;
		return -EINVAL;
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

	dev->dpy_id = KGI_INVALID_DISPLAY;
	dev->id = KGI_INVALID_DEVICE;
}

static void kgi_reattach_device(kgi_s_t id)
{
	if (kgidevice[id]) {

		KRN_DEBUG(2, "reattaching device %i", id);
	}
}















static inline kgi_u_t kgi_can_do_console(kgi_display_t *dpy)
{
	KRN_ASSERT(dpy);

	return 0;
}

static inline kgi_u_t kgi_must_do_console(kgi_display_t *dpy)
{
	kgi_u_t i;

	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {

		if ((display_map[i] == dpy->id) && kgidevice[i] &&
			(kgidevice[i]->flags & KGI_DF_CONSOLE)) {

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
	kgi_u_t i;

	KRN_DEBUG(2, "registering %s %s display with id %i", 
		dpy->vendor, dpy->model, id);

	if (! KGI_VALID_DISPLAY_ID(id)) {

		for (id = 0; (id < KGI_MAX_NR_DISPLAYS) && kgidpy[id]; id++) {
		}
		if (KGI_MAX_NR_DISPLAYS <= id) {

			return -ENOMEM;
		}
		KRN_DEBUG(2, "auto-assigned new id %i", id);
	}

	if (!KGI_VALID_DISPLAY_ID(id) || !dpy ||
		(kgidpy[id] && kgidpy[id]->graphic)) {

		KRN_DEBUG(0, "can't replace display %i", id);
		return -EINVAL;
	}

	dpy->id = id;
	dpy->graphic = 0;
	dpy->prev = kgidpy[id];
	kgidpy[dpy->id] = dpy;

	if (kgi_must_do_console(dpy) && !kgi_can_do_console(dpy)) {

		KRN_DEBUG(1, "new display has to but can't do console");
		kgidpy[id] = dpy->prev;
		dpy->prev = NULL;
		dpy->id = -1;
		return -EINVAL;
	}

	prev = dpy->prev;
	while (prev) {

		(prev->IncRefcount)(prev);
		prev = prev->prev;
	}

	for (i = 0; i < KGI_MAX_NR_DEVICES; i++) {

		if (display_map[i] == id) {

			kgi_reattach_device(i);
		}
	}

	KRN_NOTICE("display %i: %s %s registered", dpy->id,
		dpy->vendor, dpy->model);
	return KGI_EOK;
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

			kgi_reattach_device(i);
		}
	}

	prev = kgidpy[dpy->id];
	while (prev) {

		(prev->DecRefcount)(prev);
		prev = prev->prev;
	}

	KRN_NOTICE("display %i: %s %s unregistered", dpy->id,
		dpy->vendor, dpy->model);
	dpy->id   = -1;
	dpy->prev = NULL;
}
	


kgi_error_t kgi_unmap_device(kgi_u_t dev_id)
{
	kgi_device_t *dev;
	kgi_display_t *dpy;

	KRN_ASSERT(KGI_VALID_DEVICE_ID(dev_id));
	KRN_ASSERT(kgidevice[dev_id]);
	KRN_ASSERT(KGI_VALID_DISPLAY_ID(display_map[dev_id]));

	if (! (KGI_VALID_DEVICE_ID(dev_id) && kgidevice[dev_id] &&
		KGI_VALID_DISPLAY_ID(display_map[dev_id]) &&
		(dpy = kgidpy[display_map[dev_id]]))) {

		KRN_DEBUG(1, "no target or display, no unmap done");
		return -EINVAL;
	}

	if (! (dev = dpy->focus)) {

		return KGI_EOK;
	}

	KRN_DEBUG(2, "unmapping device %i from display %i", dev->id, dpy->id);

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

	KRN_DEBUG(2, "mapping device %i on display %i", dev->id, dpy->id);

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

kgi_error_t kgi_display_registered(kgi_u_t dpy_id)
{
	return (KGI_VALID_DISPLAY_ID(dpy_id) && kgidpy[dpy_id]) 
		? KGI_EOK : -ENODEV;
}

/*
**	KGI manager initialization
*/

/*	All kernel services need to be accessible when kgi_init is called,
**	especially PCI-scanning, kgi_malloc_*() services. We first
**	initialize global variables and then get the services running.
*/
static inline void kgi_init_maps(kgi_u_t nr_displays, kgi_u_t nr_focuses)
{
	kgi_u_t display = 0, device;
	for (device = 0; device < CONFIG_KGII_MAX_NR_CONSOLES; device++) {

		kgi_u_t focus   = device / 16;
		kgi_u_t console = device % 16;

		if (! (KII_VALID_FOCUS_ID(focus) &&
			KII_VALID_CONSOLE_ID(console))) {

			continue;
		}

		KRN_DEBUG(3, "mapping device %i on focus %i, display %i",
			device, focus, display);

		console_map[focus][console] = device;
		graphic_map[focus][console] = device + 
			CONFIG_KGII_MAX_NR_CONSOLES;

		focus_map[device] = 
		focus_map[device + CONFIG_KGII_MAX_NR_CONSOLES] = focus;

		display_map[device] = 
		display_map[device + CONFIG_KGII_MAX_NR_CONSOLES] = display;

		if ((device % 4) == 3) {

			if (nr_displays > nr_focuses) {

				display++;
				nr_displays--;
			}
		}
		if (console == 15) {

			nr_focuses--;
			nr_displays--;
			display++;
		}
	}
}

void kgi_init(void)
{
	kgi_u_t nr_displays, nr_focuses;

	memset(display_map, 0xFF, sizeof(display_map));
	memset(focus_map,   0xFF, sizeof(focus_map));
	memset(console_map, 0xFF, sizeof(console_map));
	memset(graphic_map, 0xFF, sizeof(graphic_map));

	memset(kgidpy, 0, sizeof(kgidpy));
	memset(kgidevice, 0, sizeof(kgidevice));

	nr_displays = 0;
#ifdef	CONFIG_KGI_DPY_I386
	nr_displays = dpy_i386_init(nr_displays, CONFIG_KGI_DISPLAYS);
#endif
#ifdef	CONFIG_KGI_DPY_NULL
	nr_displays = dpy_null_init(nr_displays, CONFIG_KGI_DISPLAYS);
#endif
	KRN_DEBUG(1, "%i displays initialized", nr_displays);


	nr_focuses  = focus_init();

	kgi_init_maps(nr_displays, nr_focuses);

	dev_console_init();
#ifdef CONFIG_KGI_DEV_GRAPHIC
	dev_graphic_init();
#endif
	/* !!!	vcs_init();	*/
}

void do_blank_screen(void)
{
}

void do_unblank_screen(void)
{
}

/*
**	exported symbols
*/
#ifdef EXPORT_SYMTAB
#include <linux/config.h>
#include <linux/module.h>

/*
**	KGI manager interface
*/
EXPORT_SYMBOL(kgi_register_display);
EXPORT_SYMBOL(kgi_unregister_display);
EXPORT_SYMBOL(kgi_register_device);
EXPORT_SYMBOL(kgi_unregister_device);

#endif	/* #ifdef EXPORT_SYMTAB */

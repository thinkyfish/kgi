/* ----------------------------------------------------------------------------
**	KGC core render
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Nicholas Souchu
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
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_render.h>

typedef struct {
	int static_alloc;
	render_t r;
} render_alloc_t;

typedef struct {
	int static_alloc;
	render_driver_t *drv;
} driver_register_t;

/* Render classes are registered to displays. Render instances are
 * allocated to devices.
 */
static driver_register_t kgc_render_drivers[KGI_MAX_NR_DISPLAYS];
static render_alloc_t  kgc_renders[KGI_MAX_NR_DEVICES];

void kgc_render_init(void)
{
	bzero(kgc_render_drivers, sizeof(kgc_render_drivers));
	bzero(kgc_renders, sizeof(kgc_renders));
}

/*
 * Allocate a new render instance. The driver (render class) has already been
 * registered to the display.
 * The render instance is assigned to the given KII consoles.
 * If the render is statically allocated, then let the caller allocate the render
 * meta.
 */
render_t kgc_render_alloc(kgi_u_t devid, render_t static_render)
{
	render_t r;
	kgi_u_t display;

	/* One can't allocated the console 0 'cause reserved to
	 * boot messages and is allocated statically.
	 */
	if (!KII_VALID_CONSOLE_ID(devid))
		return NULL;

	if (kgc_renders[devid].r) {
		KRN_ERROR("Render already allocated to %d", devid);
		return NULL;
	}

	display = display_map[devid];

	if (!kgc_render_drivers[display].drv) {
		KRN_ERROR("No render class registered to display %d", display);
		return NULL;
	}

	if (!static_render)
		r = (render_t)kobj_create((kobj_class_t)kgc_render_drivers[display].drv,
					  M_KGI, M_WAITOK);
	else
		r = static_render;

	if (!r) {
		KRN_ERROR("Could not create render %d", devid);
		return NULL;
	}

	if (!static_render) {
		if (!(r->meta = kgi_kmalloc(kgc_render_drivers[display].drv->size))) {
			KRN_ERROR("Could not allocate render meta %d", devid);
		}
	}

	kgc_renders[devid].r = r;
	kgc_renders[devid].static_alloc = (static_render) ? 1 : 0;
	r->devid = devid;

	return r;
}

void kgc_render_release(kgi_u_t devid)
{
	if (!KII_VALID_CONSOLE_ID(devid))
		return;

	if (!kgc_renders[devid].r) {
		KRN_ERROR("Render not allocated to %d", devid);
		return;
	}

	if (!kgc_renders[devid].static_alloc) {
		kgi_kfree(kgc_renders[devid].r->meta);
		kobj_delete((kobj_t)kgc_renders[devid].r, M_KGI);
	}

	kgc_renders[devid].r = NULL;
	kgc_renders[devid].static_alloc = 0;
}

kgi_error_t kgc_render_register(render_driver_t *driver, kgi_u_t display,
				kgi_u8_t already_allocated)
{
	if (!KGI_VALID_DISPLAY_ID(display) || !driver)
		return KGI_EINVAL;

	if (kgc_render_drivers[display].drv) {
		KRN_ERROR("Render driver already registered to display %d", display);
		return KGI_EINVAL;
	}

	if (!already_allocated) {
		/* Compile the render class */
		kobj_class_compile((kobj_class_t)driver);
	} else {
		kgc_render_drivers[display].static_alloc = 1;
	}

	/* Remember the render class for the display */
	kgc_render_drivers[display].drv = driver;

	return KGI_EOK;
}

kgi_error_t kgc_render_unregister(render_driver_t *driver)
{
	kgi_u_t i, display = -1;

	if (!driver)
		return KGI_EOK;

	/* All devices of the display shall have their render freed */
	for(i=0; KGI_VALID_DEVICE_ID(i); i++) {
		if (kgc_render_drivers[display_map[i]].drv == driver) {
			display = display_map[i];
			if (kgc_renders[i].r) {
				KRN_ERROR("At least a render still allocated to device %d", i);
				return KGI_EINVAL;
			}
		}
	}

	if (display != -1) {
		/* Free the render class if not allocated statically */
		if (!kgc_render_drivers[display].static_alloc)
			kobj_class_free((kobj_class_t)driver);

		kgc_render_drivers[display].drv = NULL;
		kgc_render_drivers[display].static_alloc = 0;
	}

	return KGI_EOK;
}

render_t kgc_get_render(kgi_u_t devid)
{
	if (!KGI_VALID_CONSOLE_ID(devid))
		return NULL;

	KRN_ASSERT(kgc_renders[devid].r->devid == devid);

	return kgc_renders[devid].r;
}

void *kgc_render_meta(render_t r)
{
	return (r->meta);
}


kgi_console_t *kgc_render_cons(render_t r)
{
	return (r->cons);
}


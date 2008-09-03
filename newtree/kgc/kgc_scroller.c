/* ----------------------------------------------------------------------------
**	KGC core scroller
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
#include <dev/kgc/kgc_scroller.h>

typedef struct {
	int static_alloc;
	scroller_t s;
} scroller_alloc_t;

typedef struct {
	int static_alloc;
	scroller_driver_t *drv;
} driver_register_t;

/* Render classes are registered to displays. Render instances are
 * allocated to devices.
 */
static driver_register_t kgc_scroller_drivers[KGI_MAX_NR_DISPLAYS];
static scroller_alloc_t  kgc_scrollers[KGI_MAX_NR_DEVICES];

void kgc_scroller_init(void)
{
	bzero(kgc_scroller_drivers, sizeof(kgc_scroller_drivers));
	bzero(kgc_scrollers, sizeof(kgc_scrollers));
}

/*
 * Allocate a new scroller instance. The driver (scroller class) has already been
 * registered to the display.
 * The scroller instance is assigned to the given KII consoles.
 * If the scroller is statically allocated, then let the caller allocate the scroller
 * meta.
 */
scroller_t kgc_scroller_alloc(kgi_u_t devid, scroller_t static_scroller)
{
	scroller_t s;
	kgi_u_t display;

	/* One can't allocated the console 0 'cause reserved to
	 * boot messages and is allocated statically.
	 */
	if (!KII_VALID_CONSOLE_ID(devid))
		return NULL;

	if (kgc_scrollers[devid].s) {
		KRN_ERROR("Render already allocated to %d", devid);
		return NULL;
	}

	display = display_map[devid];

	if (!kgc_scroller_drivers[display].drv) {
		KRN_ERROR("No scroller class registered to display %d", display);
		return NULL;
}

	if (!static_scroller)
		s = (scroller_t)kobj_create((kobj_class_t)kgc_scroller_drivers[display].drv,
					  M_KGI, M_WAITOK);
	else
		s = static_scroller;

	if (!s) {
		KRN_ERROR("Could not create scroller %d", devid);
		return NULL;
	}

	if (!static_scroller) {
		if (!(s->meta = kgi_kmalloc(kgc_scroller_drivers[display].drv->size))) {
			KRN_ERROR("Could not allocate scroller meta %d", devid);
		}
	}

	kgc_scrollers[devid].s = s;
	kgc_scrollers[devid].static_alloc = (static_scroller) ? 1 : 0;
	s->devid = devid;

	return s;
}

void kgc_scroller_release(kgi_u_t devid)
{
	if (!KII_VALID_CONSOLE_ID(devid))
		return;

	if (!kgc_scrollers[devid].s) {
		KRN_ERROR("Render not allocated to %d", devid);
		return;
	}

	if (!kgc_scrollers[devid].static_alloc) {
		kgi_kfree(kgc_scrollers[devid].s->meta);
		kobj_delete((kobj_t)kgc_scrollers[devid].s, M_KGI);
	}

	kgc_scrollers[devid].s = NULL;
	kgc_scrollers[devid].static_alloc = 0;
}

kgi_error_t kgc_scroller_register(scroller_driver_t *driver, kgi_u_t display,
				kgi_u8_t already_allocated)
{
	if (!KGI_VALID_DISPLAY_ID(display) || !driver)
		return KGI_EINVAL;

	if (kgc_scroller_drivers[display].drv) {
		KRN_ERROR("Render driver already registered to display %d", display);
		return KGI_EINVAL;
	}

	if (!already_allocated) {
		/* Compile the scroller class */
		kobj_class_compile((kobj_class_t)driver);
	} else {
		kgc_scroller_drivers[display].static_alloc = 1;
	}

	/* Remember the scroller class for the display */
	kgc_scroller_drivers[display].drv = driver;

	return KGI_EOK;
}

kgi_error_t kgc_scroller_unregister(scroller_driver_t *driver)
{
	kgi_u_t i, display = -1;

	if (!driver)
		return KGI_EOK;

	/* All devices of the display shall have their scroller freed */
	for(i=0; KGI_VALID_DEVICE_ID(i); i++) {
		if (kgc_scroller_drivers[display_map[i]].drv == driver) {
			display = display_map[i];
			if (kgc_scrollers[i].s) {
				KRN_ERROR("At least a scroller still allocated to device %d", i);
				return KGI_EINVAL;
			}
		}
	}

	if (display != -1) {
		/* Free the scroller class if not allocated statically */
		if (!kgc_scroller_drivers[display].static_alloc)
			kobj_class_free((kobj_class_t)driver);

		kgc_scroller_drivers[display].drv = NULL;
		kgc_scroller_drivers[display].static_alloc = 0;
	}

	return KGI_EOK;
}

scroller_t kgc_get_scroller(kgi_u_t devid)
{
	if (!KGI_VALID_CONSOLE_ID(devid))
		return NULL;

	KRN_ASSERT(kgc_scrollers[devid].s->devid == devid);

	return kgc_scrollers[devid].s;
}

void *kgc_scroller_meta(scroller_t s)
{
	return (s->meta);
}


kgi_console_t *kgc_scroller_cons(scroller_t s)
{
	return (s->cons);
}


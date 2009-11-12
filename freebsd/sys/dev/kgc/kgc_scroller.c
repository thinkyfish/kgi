/*-
 * Copyright (c) 2003 Nicholas Souchu
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
 * KGC core scroller
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

/*
 * Render classes are registered to displays. Render instances are
 * allocated to devices.
 */
static driver_register_t kgc_scroller_drivers[KGI_MAX_NR_DISPLAYS];
static scroller_alloc_t  kgc_scrollers[KGI_MAX_NR_DEVICES];

void 
kgc_scroller_init(void)
{

	bzero(kgc_scroller_drivers, sizeof(kgc_scroller_drivers));
	bzero(kgc_scrollers, sizeof(kgc_scrollers));
}

/*
 * Allocate a new scroller instance. The driver (scroller class) has already 
 * been registered to the display.
 * The scroller instance is assigned to the given KII consoles.
 * If the scroller is statically allocated, then let the caller allocate the 
 * scroller meta.
 */
scroller_t 
kgc_scroller_alloc(kgi_u_t devid, scroller_t static_scroller)
{
	scroller_t s;
	kgi_u_t display;

	/*
	 * console0 can not be allocated because it is reserved boot messages
	 * and is already statically allocated.
	 */
	if (KII_VALID_CONSOLE_ID(devid) == 0)
		return (NULL);

	if (kgc_scrollers[devid].s) {
		KGI_ERROR("Render already allocated to %d", devid);
		return (NULL);
	}

	display = display_map[devid];

	if (kgc_scroller_drivers[display].drv == NULL) {
		KGI_ERROR("No scroller class registered to display %d", display);
		return (NULL);
	}

	if (static_scroller == NULL) {
		s = (scroller_t)kobj_create((kobj_class_t)
			kgc_scroller_drivers[display].drv, M_KGI, M_WAITOK);
	} else
		s = static_scroller;

	if (s == NULL) {
		KGI_ERROR("Could not create scroller %d", devid);
		return (NULL);
	}

	if (static_scroller == NULL) {
		s->meta = kgi_kmalloc(kgc_scroller_drivers[display].drv->size);
		if (s->meta == NULL)
			KGI_ERROR("Could not allocate scroller meta %d", devid);
	}

	kgc_scrollers[devid].s = s;
	kgc_scrollers[devid].static_alloc = (static_scroller) ? 1 : 0;
	s->devid = devid;

	return (s);
}

void 
kgc_scroller_release(kgi_u_t devid)
{

	if (!KII_VALID_CONSOLE_ID(devid))
		return;

	if (kgc_scrollers[devid].s == NULL) {
		KGI_ERROR("Render not allocated to %d", devid);
		return;
	}

	if (kgc_scrollers[devid].static_alloc == 0) {
		kgi_kfree(kgc_scrollers[devid].s->meta);
		kobj_delete((kobj_t)kgc_scrollers[devid].s, M_KGI);
	}

	kgc_scrollers[devid].s = NULL;
	kgc_scrollers[devid].static_alloc = 0;
}

kgi_error_t 
kgc_scroller_register(scroller_driver_t *driver, kgi_u_t display,
		kgi_u8_t already_allocated)
{

	if (!KGI_VALID_DISPLAY_ID(display) || driver == NULL)
		return (KGI_EINVAL);

	if (kgc_scroller_drivers[display].drv) {
		KGI_ERROR("Render driver already registered to display %d", 
			  display);
		return (KGI_EINVAL);
	}

	if (already_allocated == 0) {
		/* Compile the scroller class */
		kobj_class_compile((kobj_class_t)driver);
	} else 
		kgc_scroller_drivers[display].static_alloc = 1;

	/* Remember the scroller class for the display */
	kgc_scroller_drivers[display].drv = driver;

	return (KGI_EOK);
}

kgi_error_t 
kgc_scroller_unregister(scroller_driver_t *driver)
{
	kgi_u_t i, display = -1;

	if (driver == NULL)
		return (KGI_EOK);

	/* All devices of the display shall have their scroller freed */
	for (i = 0; KGI_VALID_DEVICE_ID(i); i++) {
		if (kgc_scroller_drivers[display_map[i]].drv == driver) {
			display = display_map[i];
			if (kgc_scrollers[i].s) {
				KGI_ERROR("At least a scroller still allocated to device %i",
						  i);
				return (KGI_EINVAL);
			}
		}
	}

	if (display != -1) {
		/* Free the scroller class if not allocated statically */
		if (kgc_scroller_drivers[display].static_alloc == 0)
			kobj_class_free((kobj_class_t)driver);

		kgc_scroller_drivers[display].drv = NULL;
		kgc_scroller_drivers[display].static_alloc = 0;
	}

	return (KGI_EOK);
}

scroller_t 
kgc_get_scroller(kgi_u_t devid)
{

	if (!KGI_VALID_CONSOLE_ID(devid))
		return (NULL);

	KGI_ASSERT(kgc_scrollers[devid].s->devid == devid);

	return (kgc_scrollers[devid].s);
}

void *
kgc_scroller_meta(scroller_t s)
{

	return (s->meta);
}

kgi_console_t *
kgc_scroller_cons(scroller_t s)
{

	return (s->cons);
}


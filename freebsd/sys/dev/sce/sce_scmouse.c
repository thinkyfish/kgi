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
 * KII syscons mouse emulation input.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"
#include "opt_syscons.h"

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	1
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/consio.h>
#include <sys/fbio.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mouse.h>
#include <sys/proc.h>
#include <sys/random.h>
#include <sys/signalvar.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/tty.h>

#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>

#include <dev/kgc/kgc_emuxterm.h>
#include <dev/kgc/kgc_emudumb.h>

#include <dev/sce/sce_syscons.h>

struct sce_mice {
	kii_u_t		last;
	kii_input_t	input;
	kii_u_t		opened;
};

static struct sce_mice sce_mouse[KII_MAX_NR_FOCUSES];

/* Prototypes. */
static int mouse_event(struct sce_mice *mouse, mouse_info_t *info);

/* Mouse devsw. */
static int sce_ctlclose(struct cdev *dev, int flag, int mode, 
		struct thread *td);
static int sce_ctlioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag,
		struct thread *td);
static int sce_ctlopen(struct cdev *dev, int flag, int mode, struct thread *td);

static struct cdevsw scectl_cdevsw = {
	.d_flags 	= D_NEEDGIANT,
	.d_close 	= sce_ctlclose,
	.d_ioctl 	= sce_ctlioctl,
	.d_open	 	= sce_ctlopen,
	.d_version  = D_VERSION,
	.d_name 	= "scectl"
};

static int
mouse_event(struct sce_mice *mouse, mouse_info_t *info)
{
	kii_event_t event;

	bzero(&event, sizeof(event));
	event.any.focus  = mouse->input.focus;
	event.any.device = mouse->input.id;

	switch (info->operation) {
	case MOUSE_ACTION: /* Fall thru. */
	case MOUSE_MOTION_EVENT:
		event.pmove.size = sizeof(kii_pmove_event_t);

		event.pmove.dummy[0] = event.pmove.dummy[1] = 0;

		event.pmove.x = info->u.data.x;
		event.pmove.y = info->u.data.y;
		event.pmove.wheel = info->u.data.z;

		event.pbutton.type = KII_EV_PTR_RELATIVE;

		break;
	case MOUSE_BUTTON_EVENT:
		if ((info->u.event.id & MOUSE_BUTTONS) == 0)
			return (EINVAL);

		if (info->u.event.value < 0)
			return (EINVAL);

		event.pbutton.size = sizeof(kii_pbutton_event_t);

		if (info->u.event.value > 0)
			event.pbutton.state |= info->u.event.id;
		else
			event.pbutton.state &= ~info->u.event.id;

		event.pbutton.button = mouse->last ^ event.pbutton.state;

		mouse->last = event.pbutton.state;

		event.pbutton.type = (event.pbutton.button & event.pbutton.state)
			? KII_EV_PTR_BUTTON_PRESS : KII_EV_PTR_BUTTON_RELEASE;
		break;
	default:
		break;
	}

	if (event.pbutton.type && (mouse->input.report & 
			(1 << event.pbutton.type))) {		
		KGI_DEBUG(10, "%s", 
			  (event.pbutton.type == KII_EV_PTR_BUTTON_PRESS) ?
			  "KII_EV_PTR_BUTTON_PRESS" :
			  "KII_EV_PTR_BUTTON_RELEASE");

		kii_handle_input(&event);
	}

	return (0);
}

static int
sce_ctlclose(struct cdev *dev, int flag, int mode, struct thread *td)
{

	KGI_DEBUG(2, "sce_ctlclose: dev:%s, vty:,%d\n", 
			devtoname(dev), dev2unit(dev));

	sce_mouse[dev2unit(dev)].opened = 0;

	return (0);
}

static int
sce_ctlioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag,
			 struct thread *td)
{
	struct sce_mice *mouse;
	mouse_info_t *info;
	int error;

	mouse = &sce_mouse[dev2unit(dev)];	
	error = 0;

	switch (cmd) {
	case CONS_MOUSECTL: /* Control mouse arrow. */
		info = (mouse_info_t*)data;

		random_harvest(info, sizeof(mouse_info_t), 2, 0, RANDOM_MOUSE);

		switch (info->operation) {
		case MOUSE_MODE: /* Fall thru. */
		case MOUSE_SHOW: /* Fall thru. */
		case MOUSE_HIDE: /* Fall thru. */
		case MOUSE_MOVEABS: /* Fall thru. */
		case MOUSE_MOVEREL: /* Fall thru. */
		case MOUSE_GETINFO: /* Fall thru. */
			return (EINVAL);
			/* Send out mouse event on /dev/sysmouse. */
		case MOUSE_ACTION: /* Fall thru. */
		case MOUSE_MOTION_EVENT:
			error = mouse_event(mouse, info);
			break;
		case MOUSE_BUTTON_EVENT:
			if (((info->u.event.id & MOUSE_BUTTONS) == 0) ||
			    (info->u.event.value < 0))
				return (EINVAL);
				
			error = mouse_event(mouse, info);
			break;
		case MOUSE_MOUSECHAR: /* Fall thru. */
		default:
			return (EINVAL);
		}

		return (error);
	}

	return (ENOIOCTL);
}

static int
sce_ctlopen(struct cdev *dev, int flag, int mode, struct thread *td)
{
	struct sce_mice *mouse;

	KGI_DEBUG(2, "sce_ctlopen: dev:%s, vty:%d\n",
			devtoname(dev), dev2unit(dev));

	mouse = &sce_mouse[dev2unit(dev)];

	if (mouse->opened)
		return (EBUSY);

	snprintf(mouse->input.vendor, KII_MAX_VENDOR_STRING, "FreeBSD");
	snprintf(mouse->input.model, KII_MAX_VENDOR_STRING, "consolectl");
	
	mouse->input.focus = KII_INVALID_FOCUS;
	mouse->input.id = KII_INVALID_DEVICE;
	mouse->input.events = KII_EM_POINTER & ~KII_EM_PTR_ABSOLUTE;
	mouse->input.report = KII_EM_PTR_RELATIVE | KII_EM_PTR_BUTTON;
	mouse->input.priv.priv_ptr = mouse;
	
	if (kii_register_input(dev2unit(dev), &mouse->input, 0))
		KGI_ERROR("Could not register sce_mouse %d", dev2unit(dev));

	mouse->opened = 1;

	return (0);
}

int 
sce_mouse_init()
{
	int focus;

	bzero(sce_mouse, sizeof(sce_mouse));
	make_dev(&scectl_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, "consolectl");
	for (focus = 1; focus < KII_MAX_NR_FOCUSES; focus++) {
		make_dev(&scectl_cdevsw, focus, UID_ROOT, GID_WHEEL, 0600,
				 "consolectl" "%d", focus);
	}

	return (0);
}

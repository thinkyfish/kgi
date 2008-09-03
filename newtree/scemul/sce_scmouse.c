/* ---------------------------------------------------------------------------
**      KII syscons mouse emulation input
** ---------------------------------------------------------------------------
**      Copyright (C)   2003       Nicholas Souchu
**
**      This file is distributed under the terms and conditions of the
**      MIT/X public license. Please see the file COPYRIGHT.MIT included
**      with this software for details of these terms and conditions.
**      Alternatively you may distribute this file under the terms and
**      conditions of the GNU General Public License. Please see the file
**      COPYRIGHT.GPL included with this software for details of these terms
**      and conditions.
** -------------------------------------------------------------------------
**
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

#include <dev/scemul/sce_syscons.h>

typedef struct {
	kii_u_t		last;
	kii_input_t	input;
	kii_u_t		opened;
} sce_mouse_t;

static sce_mouse_t sce_mouses[KII_MAX_NR_FOCUSES];

static int
mouse_event(sce_mouse_t *mouse, mouse_info_t *info)
{
	kii_event_t event;

	bzero(&event, sizeof(event));

	event.any.focus  = mouse->input.focus;
	event.any.device = mouse->input.id;

	switch (info->operation) {
	case MOUSE_ACTION:
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
			return EINVAL;

		if (info->u.event.value < 0)
			return EINVAL;

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

	if (event.pbutton.type && (mouse->input.report & (1 << event.pbutton.type))) {
		
		KRN_DEBUG(2, "%s", 
			  (event.pbutton.type == KII_EV_PTR_BUTTON_PRESS)
			  ? "KII_EV_PTR_BUTTON_PRESS" 
			  : "KII_EV_PTR_BUTTON_RELEASE");

		kii_handle_input(&event);
	}

	return 0;
}

static int
sce_ctlopen(struct cdev *dev, int flag, int mode, struct thread *td)
{
	sce_mouse_t *mouse;

	KRN_DEBUG(2, "sce_ctlopen: dev:%s, vty:%d\n", devtoname(dev), minor(dev));

	mouse = &sce_mouses[minor(dev)];

	if (mouse->opened)
		return (EBUSY);

	snprintf(mouse->input.vendor, KII_MAX_VENDOR_STRING, "FreeBSD");
	snprintf(mouse->input.model, KII_MAX_VENDOR_STRING, "consolectl");
	
	mouse->input.focus = KII_INVALID_FOCUS;
	mouse->input.id = KII_INVALID_DEVICE;
	mouse->input.events = KII_EM_POINTER & ~KII_EM_PTR_ABSOLUTE;
	mouse->input.report = KII_EM_PTR_RELATIVE | KII_EM_PTR_BUTTON;
	mouse->input.priv.priv_ptr = mouse;
	
	if (kii_register_input(minor(dev), &mouse->input))
		KRN_ERROR("Could not register sce_mouse %d", minor(dev));

	mouse->opened = 1;

	return (0);
}

static int
sce_ctlclose(struct cdev *dev, int flag, int mode, struct thread *td)
{

	KRN_DEBUG(2, "sce_ctlclose: dev:%s, vty:,%d\n", devtoname(dev), minor(dev));

	sce_mouses[minor(dev)].opened = 0;

	return (0);
}

static int
sce_ctlioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag, struct thread *td)
{
	sce_mouse_t *mouse = &sce_mouses[minor(dev)];
	mouse_info_t *info;
	int error = 0;

	switch (cmd) {

	case CONS_MOUSECTL:		/* control mouse arrow */
		info = (mouse_info_t*)data;

		random_harvest(info, sizeof(mouse_info_t), 2, 0, RANDOM_MOUSE);

		switch (info->operation) {
		case MOUSE_MODE:
		case MOUSE_SHOW:
		case MOUSE_HIDE:
		case MOUSE_MOVEABS:
		case MOUSE_MOVEREL:
		case MOUSE_GETINFO:
			return (EINVAL);

			/* send out mouse event on /dev/sysmouse */
		case MOUSE_ACTION:
		case MOUSE_MOTION_EVENT:

			error = mouse_event(mouse, info);
			break;

		case MOUSE_BUTTON_EVENT:

			if (((info->u.event.id & MOUSE_BUTTONS) == 0) ||
			    (info->u.event.value < 0))
				return (EINVAL);

			error = mouse_event(mouse, info);
			break;

		case MOUSE_MOUSECHAR:
		default:
			return (EINVAL);
		}

		return error;
	}

	return (ENOIOCTL);
}

static struct cdevsw scectl_cdevsw = {
	.d_open =	sce_ctlopen,
	.d_close =	sce_ctlclose,
	.d_ioctl =	sce_ctlioctl,
	.d_name =	"scectl",
	.d_flags =	D_TTY | D_NEEDGIANT,
	.d_version =	D_VERSION
};

int sce_mouse_init()
{
	int focus;

	bzero(sce_mouses, sizeof(sce_mouses));

	make_dev(&scectl_cdevsw, 0, UID_ROOT, GID_WHEEL, 0600, "consolectl");

	for(focus=1; focus<KII_MAX_NR_FOCUSES; focus++)
	{
		make_dev(&scectl_cdevsw, focus, UID_ROOT, GID_WHEEL, 0600, "consolectl" "%d", focus);
	}

	return (0);
}

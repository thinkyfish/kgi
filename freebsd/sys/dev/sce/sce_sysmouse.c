/*-
 * Copyright (c) 1999 Kazutaka YOKOTA <yokota@zodiac.mech.utsunomiya-u.ac.jp>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * From FreeBSD: sys/dev/syscons/sysmouse.c -r196539M
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"
#include "opt_syscons.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/priv.h>
#include <sys/serial.h>
#include <sys/tty.h>
#include <sys/kernel.h>
#include <sys/consio.h>
#include <sys/mouse.h>

#include <dev/kgi/system.h>
#include <dev/kgi/debug.h>
#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#include <dev/kii/kii.h>
#include <dev/kgc/kgc_console.h>
#include <dev/kgc/kgc_emuxterm.h>
#include <dev/kgc/kgc_emudumb.h>

#include <dev/sce/sce_syscons.h>

#ifndef SC_NO_SYSMOUSE

typedef struct {
	struct tty *tty;
	int level;	/* sysmouse protocol level. */
	mousestatus_t status;
}sce_sysmouse;

static tsw_close_t		sce_smclose;
static tsw_ioctl_t		sce_smioctl;
static tsw_param_t		sce_smparam;

static struct ttydevsw scesm_ttydevsw = {
	.tsw_flags		= TF_NOPREFIX,
	.tsw_close		= sce_smclose,
	.tsw_ioctl		= sce_smioctl,
	.tsw_param		= sce_smparam,

};

static sce_sysmouse sysmouse;

static void
sce_smclose(struct tty *tp)
{
	int s;

	s = spltty();
	sysmouse.level = 0;
	sysmouse.tty = NULL;
	splx(s);

	return;
}

static int
sce_smioctl(struct tty *tp, u_long cmd, caddr_t data, struct thread *td)
{
	mousehw_t *hw;
	mousemode_t *mode;
	int s;

	switch (cmd) {
	case MOUSE_GETHWINFO: /* Get device information. */
		hw = (mousehw_t *)data;
		hw->buttons = 10; /* XXX unknown. */
		hw->iftype = MOUSE_IF_SYSMOUSE;
		hw->type = MOUSE_MOUSE;
		hw->model = MOUSE_MODEL_GENERIC;
		hw->hwid = 0;
		return (0);
	case MOUSE_GETMODE:	/* Get protocol/mode. */
		mode = (mousemode_t *)data;
		mode->level = sysmouse.level;
		switch (mode->level) {
		case 0: /* Emulate MouseSystems protocol. */
			mode->protocol = MOUSE_PROTO_MSC;
			mode->rate = -1;		/* Unknown. */
			mode->resolution = -1;	/* Unknown. */
			mode->accelfactor = 0;	/* Disabled. */
			mode->packetsize = MOUSE_MSC_PACKETSIZE;
			mode->syncmask[0] = MOUSE_MSC_SYNCMASK;
			mode->syncmask[1] = MOUSE_MSC_SYNC;
			break;
		case 1: /* sysmouse protocol. */
			mode->protocol = MOUSE_PROTO_SYSMOUSE;
			mode->rate = -1;
			mode->resolution = -1;
			mode->accelfactor = 0;
			mode->packetsize = MOUSE_SYS_PACKETSIZE;
			mode->syncmask[0] = MOUSE_SYS_SYNCMASK;
			mode->syncmask[1] = MOUSE_SYS_SYNC;
			break;
		}
		return (0);
	case MOUSE_SETMODE:	/* Set protocol/mode. */
		mode = (mousemode_t *)data;
		if (mode->level == -1)
			; 	/* Don't change the current setting. */
		else if ((mode->level < 0) || (mode->level > 1))
			return (EINVAL);
		else
			sysmouse.level = mode->level;
		return (0);
	case MOUSE_GETLEVEL: /* Get operation level. */
		*(int *)data = sysmouse.level;
		return (0);
	case MOUSE_SETLEVEL: /* Set operation level. */
		if ((*(int *)data  < 0) || (*(int *)data > 1))
			return (EINVAL);
		sysmouse.level = *(int *)data;
		return (0);
	case MOUSE_GETSTATUS: /* Get accumulated mouse events. */
		s = spltty();
		*(mousestatus_t *)data = sysmouse.status;
		sysmouse.status.flags = 0;
		sysmouse.status.obutton = sysmouse.status.button;
		sysmouse.status.dx = 0;
		sysmouse.status.dy = 0;
		sysmouse.status.dz = 0;
		splx(s);
		return (0);
#ifdef notyet
	case MOUSE_GETVARS:	/* Get internal mouse variables. */
	case MOUSE_SETVARS:	/* Set internal mouse variables. */
		return (ENODEV);
#endif
	case MOUSE_READSTATE: /* Read status from the device. */
	case MOUSE_READDATA:  /* Read data from the device.   */
		return (ENODEV);
	}

	return (ENOIOCTL);
}

static int
sce_smparam(struct tty *tp, struct termios *t)
{

	/*
	 * Set the output baud rate to zero. The mouse device supports
	 * no output, so we don't want to waste buffers.
	 */
	t->c_ispeed = TTYDEF_SPEED;
	t->c_ospeed = B0;
	
	return (0);
}

void 
sce_sysmouse_event(kii_event_t *ev)
{	
	
	/* MOUSE_BUTTON?DOWN -> MOUSE_MSC_BUTTON?UP */
	static int butmap[8] = {
	    MOUSE_MSC_BUTTON1UP | MOUSE_MSC_BUTTON2UP | MOUSE_MSC_BUTTON3UP,
	    MOUSE_MSC_BUTTON2UP | MOUSE_MSC_BUTTON3UP,
	    MOUSE_MSC_BUTTON1UP | MOUSE_MSC_BUTTON3UP,
	    MOUSE_MSC_BUTTON3UP,
	    MOUSE_MSC_BUTTON1UP | MOUSE_MSC_BUTTON2UP,
	    MOUSE_MSC_BUTTON2UP,
	    MOUSE_MSC_BUTTON1UP,
	    0,
	};
	/* XXX FIXME */
	sce_sysmouse *sysm;
	u_char buf[8];
	int i, x, y, z;

	sysm = &sysmouse;

	switch (ev->pbutton.type) {
	case KII_EV_PTR_RELATIVE:
        sysm->status.button = 0;
		x = ev->pmove.x;
		y = ev->pmove.y;
		z = ev->pmove.wheel;
		break;
	case KII_EV_PTR_BUTTON_PRESS: /* Fall thru. */
	case KII_EV_PTR_BUTTON_RELEASE:
		x = y = z = 0;
		sysm->status.button |= ev->pbutton.state;
		break;
	default:
		return;
	}

	sysm->status.dx += x;
	sysm->status.dy += y;
	sysm->status.dz += z;
	sysm->status.flags |= ((x || y || z) ? MOUSE_POSCHANGED : 0)
			      | (ev->pbutton.button);
	if (sysm->status.flags == 0)
		return;

	if ((sysm->tty == NULL) || !tty_opened(sysm->tty))
		return;

	/* The first five bytes are compatible with MouseSystems' */
	buf[0] = MOUSE_MSC_SYNC
		 | butmap[sysm->status.button & MOUSE_STDBUTTONS];
	x = imax(imin(x, 255), -256);
	buf[1] = x >> 1;
	buf[3] = x - buf[1];
	y = -imax(imin(y, 255), -256);
	buf[2] = y >> 1;
	buf[4] = y - buf[2];

	tty_lock(sysm->tty);
	for (i = 0; i < MOUSE_MSC_PACKETSIZE; ++i)
		ttydisc_rint(sysm->tty, (char)buf[i], 0);
	
	if (sysm->level >= 1) {		
		/* Extended part. */
        z = imax(imin(z, 127), -128);
        buf[5] = (z >> 1) & 0x7f;
        buf[6] = (z - (z >> 1)) & 0x7f;
        /* Buttons 4-10 */
        buf[7] = (~sysm->status.button >> 3) & 0x7f;
        for (i = MOUSE_MSC_PACKETSIZE; i < MOUSE_SYS_PACKETSIZE; ++i)
			ttydisc_rint(sysm->tty, (char)buf[i], 0);				
	}	

	ttydisc_rint_done(sysm->tty);
	tty_unlock(sysm->tty);

	return;
}

int 
sce_sysmouse_init(void)
{

	bzero(&sysmouse, sizeof(sce_sysmouse));	
	sysmouse.tty = tty_alloc(&scesm_ttydevsw, NULL);
	tty_makedev(sysmouse.tty, NULL, "sysmouse");

	return (0);
}
SYSINIT(sce_sysmouse, SI_SUB_DRIVERS, SI_ORDER_MIDDLE, sce_sysmouse_init, NULL);

#endif /* !SC_NO_SYSMOUSE */

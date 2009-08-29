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
 *
 * From FreeBSD: /head/sys/dev/syscons/sysmouse.c -r193018
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"
#include "opt_syscons.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/ttydisc.h>
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

#define SC_MOUSE 	128		/* minor number */

typedef struct {
	struct tty	*tty;
	int		level;	/* sysmouse protocol level */
	mousestatus_t	status;
} sce_sysmouse_t;

static tsw_close_t	scesm_close;
static tsw_ioctl_t	scesm_ioctl;
static tsw_open_t	scesm_open;
static tsw_param_t	scesm_param;

static struct ttydevsw scesm_ttydevsw = {
	.tsw_flags	= TF_NOPREFIX | D_NEEDGIANT,
	.tsw_close	= scesm_close,
	.tsw_ioctl	= scesm_ioctl,
	.tsw_open	= scesm_open,
	.tsw_param	= scesm_param
};

static sce_sysmouse_t sce_sysmouse;

static int
scesm_param(struct tty *tp, struct termios *t)
{
	/*
	 * Set the output baud rate to zero. The mouse device supports
	 * no output, so we don't want to waste buffers.
	 */
	t->c_ispeed = TTYDEF_SPEED;
	t->c_ospeed = B0;
	
	return (0);
}
/*
// XXX
// What the fuck is all this for?
// more to outp?
static void
scesm_start(struct tty *tp)
{
	u_char buf[PCBURST];

	tty_lock(tp);
	if (!(tp->t_flags & (TF_BUSY | TF_STOPPED))) {
		tp->t_flags |= TF_BUSY;
		while (rbp->c_cc)
			q_to_b(rbp, buf, PCBURST);
		tp->t_state &= ~TS_BUSY;
	}
	tty_unlock(tp);
}*/

static int
scesm_open(struct tty *tp)
{

	if (!(tp->t_flags & TF_OPENED)) {
		tty_lock(tp);
		ttydisc_modem(tp, 1);
		tty_unlock(tp);
	} else if (tp->t_flags & TF_EXCLUDE ) 
		return (EBUSY);
	
//	tty_lock(tp);
//	return ((int)ttydisc_open(tp));
//	tty_unlock(tp);
	tty_lock(tp);
	ttydisc_open(tp);
	tty_unlock(tp);

return (0);
}

static void
scesm_close(struct tty *tp)
{
	int s;

	s = spltty();
	sce_sysmouse.level = 0;
	sce_sysmouse.tty = NULL;
	tty_lock(tp);
	ttydisc_close(tp);
	tty_unlock(tp);
	splx(s);

	return;
}

static int
scesm_ioctl(struct tty *tp, u_long cmd, caddr_t data, struct thread *td)
{
	mousehw_t *hw;
	mousemode_t *mode;
	int error;
	int s;

	switch (cmd) {
	case MOUSE_GETHWINFO:	/* get device information */
		hw = (mousehw_t *)data;
		hw->buttons = 10;		/* XXX unknown */
		hw->iftype = MOUSE_IF_SYSMOUSE;
		hw->type = MOUSE_MOUSE;
		hw->model = MOUSE_MODEL_GENERIC;
		hw->hwid = 0;
		return (0);
	case MOUSE_GETMODE:	/* get protocol/mode */
		mode = (mousemode_t *)data;
		mode->level = sce_sysmouse.level;
		switch (mode->level) {
		case 0: /* emulate MouseSystems protocol */
			mode->protocol = MOUSE_PROTO_MSC;
			mode->rate = -1;		/* unknown */
			mode->resolution = -1;	/* unknown */
			mode->accelfactor = 0;	/* disabled */
			mode->packetsize = MOUSE_MSC_PACKETSIZE;
			mode->syncmask[0] = MOUSE_MSC_SYNCMASK;
			mode->syncmask[1] = MOUSE_MSC_SYNC;
			break;
		case 1: /* sysmouse protocol */
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
	case MOUSE_SETMODE:	/* set protocol/mode */
		mode = (mousemode_t *)data;
		if (mode->level == -1)
			; 	/* don't change the current setting */
		else if ((mode->level < 0) || (mode->level > 1))
			return (EINVAL);
		else
			sce_sysmouse.level = mode->level;
		return (0);
	case MOUSE_GETLEVEL:	/* get operation level */
		*(int *)data = sce_sysmouse.level;
		return (0);
	case MOUSE_SETLEVEL:	/* set operation level */
		if ((*(int *)data  < 0) || (*(int *)data > 1))
			return (EINVAL);
		sce_sysmouse.level = *(int *)data;
		return (0);
	case MOUSE_GETSTATUS:	/* get accumulated mouse events */
		s = spltty();
		*(mousestatus_t *)data = sce_sysmouse.status;
		sce_sysmouse.status.flags = 0;
		sce_sysmouse.status.obutton = sce_sysmouse.status.button;
		sce_sysmouse.status.dx = 0;
		sce_sysmouse.status.dy = 0;
		sce_sysmouse.status.dz = 0;
		splx(s);
		return (0);
#ifdef notyet
	case MOUSE_GETVARS:	/* get internal mouse variables */
	case MOUSE_SETVARS:	/* set internal mouse variables */
		return (ENODEV);
#endif
	case MOUSE_READSTATE:	/* read status from the device */
	case MOUSE_READDATA:	/* read data from the device */
		return (ENODEV);
	}

	error = tty_ioctl(tp, cmd, data, td);
	if (error != ENOIOCTL)
		return (error);

	return (ENOTTY);
}

void sce_sysmouse_event(kii_event_t *ev)
{
	/* XXX FIXME */
	sce_sysmouse_t *sysm = &sce_sysmouse;

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
	u_char buf[8];
	int x, y, z;
	int i;

	switch (ev->pbutton.type) {
	case KII_EV_PTR_RELATIVE:
        sysm->status.button = 0;
		x = ev->pmove.x;
		y = ev->pmove.y;
		z = ev->pmove.wheel;
		break;
	case KII_EV_PTR_BUTTON_PRESS:
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

	if ((sysm->tty == NULL) || !(sysm->tty->t_flags & TF_OPENED))
		return;

	/* the first five bytes are compatible with MouseSystems' */
	buf[0] = MOUSE_MSC_SYNC
		 | butmap[sysm->status.button & MOUSE_STDBUTTONS];
	x = imax(imin(x, 255), -256);
	buf[1] = x >> 1;
	buf[3] = x - buf[1];
	y = -imax(imin(y, 255), -256);
	buf[2] = y >> 1;
	buf[4] = y - buf[2];
	tty_lock(sysm->tty);
	for (i = 0; i < MOUSE_MSC_PACKETSIZE; ++i) {
		ttydisc_rint(sysm->tty, (char)buf[i], 0);
		ttydisc_rint_done(sysm->tty);
	}
	
	tty_unlock(sysm->tty);
	
	if (sysm->level >= 1) {		
		/* extended part */
        z = imax(imin(z, 127), -128);
        buf[5] = (z >> 1) & 0x7f;
        buf[6] = (z - (z >> 1)) & 0x7f;
        /* buttons 4-10 */
        buf[7] = (~sysm->status.button >> 3) & 0x7f;
		tty_lock(sysm->tty);
        for (i = MOUSE_MSC_PACKETSIZE; i < MOUSE_SYS_PACKETSIZE; ++i) {
			ttydisc_rint(sysm->tty, (char)buf[i], 0);			
			ttydisc_rint_done(sysm->tty);
		}
		tty_unlock(sysm->tty);
	}	

	return;
}

int 
sce_sysmouse_init(void)
{

	bzero(&sce_sysmouse, sizeof(sce_sysmouse_t));	
	sce_sysmouse.tty = tty_alloc(&scesm_ttydevsw, &Giant);
	tty_makedev(sce_sysmouse.tty, NULL, "sysmouse");
//	make_dev(&scesm_cdevsw, SC_MOUSE, UID_ROOT, GID_WHEEL, 0600,
//		 "sysmouse");

	return (0);
}

SYSINIT(sce_sysmouse, SI_SUB_DRIVERS, SI_ORDER_MIDDLE, sce_sysmouse_init, NULL);

#endif /* !SC_NO_SYSMOUSE */

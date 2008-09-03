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
 * From FreeBSD: src/sys/dev/syscons/sysmouse.c,v 1.14 2003/03/03 12:15:47 phk Exp
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_kgi.h"
#include "opt_syscons.h"

#include <sys/param.h>
#include <sys/priv.h>
#include <sys/systm.h>
#include <sys/conf.h>
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

#include <dev/scemul/sce_syscons.h>

#ifndef SC_NO_SYSMOUSE

#define SC_MOUSE 	128		/* minor number */

typedef struct {

	struct tty	*tty;
	int		level;	/* sysmouse protocol level */
	mousestatus_t	status;

} sce_sysmouse_t;

static sce_sysmouse_t sce_sysmouse;

/* local variables */

static void
sce_smstart(struct tty *tp)
{
	struct clist *rbp;
	u_char buf[PCBURST];
	int s;

	s = spltty();
	if (!(tp->t_state & (TS_TIMEOUT | TS_BUSY | TS_TTSTOP))) {
		tp->t_state |= TS_BUSY;
		rbp = &tp->t_outq;
		while (rbp->c_cc)
			q_to_b(rbp, buf, PCBURST);
		tp->t_state &= ~TS_BUSY;
		ttwwakeup(tp);
	}
	splx(s);
}

static int
sce_smparam(struct tty *tp, struct termios *t)
{
	tp->t_ispeed = t->c_ispeed;
	tp->t_ospeed = t->c_ospeed;
	tp->t_cflag = t->c_cflag;
	return 0;
}

static int
sce_smopen(struct cdev *dev, int flag, int mode, struct thread *td)
{
	struct tty *tp;

	sce_sysmouse.tty = tp = dev->si_tty = ttyalloc();
	if (!(tp->t_state & TS_ISOPEN)) {
		tp->t_oproc = sce_smstart;
		tp->t_param = sce_smparam;
		tp->t_stop = nottystop;
		tp->t_dev = dev;
		ttychars(tp);
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_cflag = TTYDEF_CFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
		sce_smparam(tp, &tp->t_termios);
		ttyld_modem(tp, 1);
	} else if (tp->t_state & TS_XCLUDE && suser(td)) {
		return EBUSY;
	}

	return ttyld_open(tp, dev);
}

static int
sce_smclose(struct cdev *dev, int flag, int mode, struct thread *td)
{
	struct tty *tp;
	int s;

	tp = dev->si_tty;
	s = spltty();
	sce_sysmouse.level = 0;
	sce_sysmouse.tty = NULL;
	ttyld_close(tp, flag);
	tty_close(tp);
	splx(s);

	return 0;
}

static int
sce_smioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag, struct thread *td)
{
	struct tty *tp = dev->si_tty;
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
		return 0;

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
		return 0;

	case MOUSE_SETMODE:	/* set protocol/mode */
		mode = (mousemode_t *)data;
		if (mode->level == -1)
			; 	/* don't change the current setting */
		else if ((mode->level < 0) || (mode->level > 1))
			return EINVAL;
		else
			sce_sysmouse.level = mode->level;
		return 0;

	case MOUSE_GETLEVEL:	/* get operation level */
		*(int *)data = sce_sysmouse.level;
		return 0;

	case MOUSE_SETLEVEL:	/* set operation level */
		if ((*(int *)data  < 0) || (*(int *)data > 1))
			return EINVAL;
		sce_sysmouse.level = *(int *)data;
		return 0;

	case MOUSE_GETSTATUS:	/* get accumulated mouse events */
		s = spltty();
		*(mousestatus_t *)data = sce_sysmouse.status;
		sce_sysmouse.status.flags = 0;
		sce_sysmouse.status.obutton = sce_sysmouse.status.button;
		sce_sysmouse.status.dx = 0;
		sce_sysmouse.status.dy = 0;
		sce_sysmouse.status.dz = 0;
		splx(s);
		return 0;

#ifdef notyet
	case MOUSE_GETVARS:	/* get internal mouse variables */
	case MOUSE_SETVARS:	/* set internal mouse variables */
		return ENODEV;
#endif

	case MOUSE_READSTATE:	/* read status from the device */
	case MOUSE_READDATA:	/* read data from the device */
		return ENODEV;
	}

	error = ttioctl(tp, cmd, data, flag);
	if (error != ENOIOCTL)
		return error;

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

	if ((sysm->tty == NULL) || !(sysm->tty->t_state & TS_ISOPEN))
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
	for (i = 0; i < MOUSE_MSC_PACKETSIZE; ++i)
		ttyld_rint(sysm->tty, buf[i]);
	if (sysm->level >= 1) {
		/* extended part */
        	z = imax(imin(z, 127), -128);
        	buf[5] = (z >> 1) & 0x7f;
        	buf[6] = (z - (z >> 1)) & 0x7f;
        	/* buttons 4-10 */
        	buf[7] = (~sysm->status.button >> 3) & 0x7f;
        	for (i = MOUSE_MSC_PACKETSIZE; i < MOUSE_SYS_PACKETSIZE; ++i)
			ttyld_rint(sysm->tty, buf[i]);
	}

	return;
}

static struct cdevsw scesm_cdevsw = {
	.d_open =	sce_smopen,
	.d_close =	sce_smclose,
	.d_read =	ttyread,
	.d_ioctl =	sce_smioctl,
	.d_poll =	ttypoll,
	.d_name =	"sysmouse",
	.d_flags =	D_TTY | D_NEEDGIANT,
	.d_version =	D_VERSION
};

int sce_sysmouse_init(void) {

	bzero(&sce_sysmouse, sizeof(sce_sysmouse_t));

	make_dev(&scesm_cdevsw, SC_MOUSE, UID_ROOT, GID_WHEEL, 0600,
		 "sysmouse");

	return 0;
}

#endif /* !SC_NO_SYSMOUSE */

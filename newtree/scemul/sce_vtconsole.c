/* ----------------------------------------------------------------------------
**	KGI console driver
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**	Copyright (C)	2003		Nicholas Souchu
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

/* XXX FIXME */
#define KGC_TERM_XTERM

#ifndef KGI_DBG_LEVEL
#define	KGI_DBG_LEVEL	2
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/callout.h>
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/priv.h>
#include <sys/random.h>
#include <sys/signalvar.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/uio.h>
#include <sys/kbio.h>

#define KGI_SYS_NEED_MALLOC
#include <dev/kgi/system.h>

#include <dev/kgi/kgii.h>
#include <dev/kgi/kgi.h>
#define KII_NEED_MODIFIER_KEYSYMS
#include <dev/kii/kii.h>

#include <dev/kgc/kgc_console.h>

#include <dev/kgc/kgc_textbuf.h>
#include <dev/kgc/kgc_scroller.h>
#include <dev/kgc/kgc_render.h>
#include <dev/kgc/kgc_gfbrndr.h>

#include <dev/kgc/kgc_emuxterm.h>
#include <dev/kgc/kgc_emudumb.h>

#include <dev/scemul/sce_syscons.h>

#include "scroller_if.h"
#include "render_if.h"

sce_console_t *sce_consoles[CONFIG_KGII_MAX_NR_CONSOLES];

static int first_minor_allocated = 0;

static void handle_kii_event(kii_device_t *dev, kii_event_t *e)
{

	/* Forward to sysmouse pointer events */
	if ((1 << e->any.type) & KII_EM_POINTER) {
		sce_sysmouse_event(e);
		return;
	}

#ifdef KGC_TERM_XTERM
	xterm_handle_kii_event(dev, e);
#else
#ifndef KGC_TERM_DUMB
#error KGC_TERM_DUMB not defined
#endif
	dumb_handle_kii_event(dev, e);
#endif
}


/* !!!	This should handle assignment of registered terminal emulators later.
** !!!	For now we only enter the neccessary fields for the dumb or xterm
** !!!	parser.
*/
static void assign_parser(kgi_console_t *cons, int do_reset)
{
	KRN_ASSERT(cons);

	cons->kii.MapDevice		= kgc_map_kii;
	cons->kii.UnmapDevice	= kgc_unmap_kii;
	cons->kii.HandleEvent	= &handle_kii_event;
	cons->kii.event_mask	= KII_EM_POINTER;

#ifdef KGC_TERM_XTERM
	cons->DoWrite = &xterm_do_write;
	xterm_do_reset(cons, do_reset);
#else
#ifndef KGC_TERM_DUMB
#error KGC_TERM_DUMB not defined
#endif
	cons->DoWrite = &dumb_do_write;
	dumb_do_reset(cons, do_reset);
#endif
}

static void start(struct tty *tp)
{
	int s, len, unit = minor(tp->t_dev);
	kgi_console_t *cons = (kgi_console_t *)sce_consoles[unit];
	struct clist *rbp;
	u_char buf[PCBURST];

	if (!cons)
		return;

	s = spltty();

/*	cons->kii.flags |= KII_DF_SCROLL_LOCK; */

	kiidev_sync(&(cons->kii), KII_SYNC_LED_FLAGS);

	if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
		goto out;

	tp->t_state |= TS_BUSY;

	splx(s);

	rbp = &tp->t_outq;

	/*
	 * Call q_to_b() at spltty() to ensure that the queue is empty when
	 * the loop terminates.
	 */

	s = spltty();

	while((len = q_to_b(rbp, buf, PCBURST)) > 0)
	{
		/*
		 * We need to do this outside spl since it could be fairly
		 * expensive and we don't want our serial ports to overflow.
		 */
		splx(s);
		cons->DoWrite(cons, buf, len);
		s = spltty();
	}

	tp->t_state &= ~TS_BUSY;

	ttwwakeup(tp);

out:
	splx(s);
}

static int param(struct tty *tp, struct termios *t)
{
        tp->t_ispeed = t->c_ispeed;
        tp->t_ospeed = t->c_ospeed;
        tp->t_cflag  = t->c_cflag;

	return (0);
}

#if 0
static int stop(struct tty *tp)
{
	int unit = minor(tp->t_dev);
	kgi_console_t *cons = (kgi_console_t *)sce_consoles[unit];
	int s;

	s = spltty();

	cons->kii.flags &= ~KII_DF_SCROLL_LOCK;
	kiidev_sync(&(cons->kii), KII_SYNC_LED_FLAGS);

	splx(s);
}
#endif

static int
sce_vtopen(struct cdev *dev, int flag, int mode, struct thread *td)
{
	kgi_u_t index = minor(dev);
	kgi_console_t *cons;
	kgi_error_t err;
	struct tty *tp = NULL;
	int retval, s;

	if (index >= CONFIG_KGII_MAX_NR_CONSOLES) {
		KRN_ERROR("Too much consoles!");
		return ENXIO;
	}

	cons = (kgi_console_t *)sce_consoles[index];

	if (!cons) {

		KRN_DEBUG(2, "allocating console %i...", index);

		cons = kgi_kmalloc(sizeof(sce_console_t));
		if (!cons) {
			KRN_ERROR("failed: not enough memory");
			return (ENOMEM);
		}
		memset(cons, 0, sizeof(sce_console_t));

		/* If the minor is null, the console was not initialized */
		if (!index)
			first_minor_allocated = 1;
	}

	tp = dev->si_tty = ttyalloc();

	tp->t_oproc = start;
	tp->t_param = param;
	tp->t_stop = nottystop;
	tp->t_dev = dev;

	if ((tp->t_state & TS_ISOPEN) == 0) {
		ttychars(tp);
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_cflag = TTYDEF_CFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
		param(tp, &tp->t_termios);
		ttyld_modem(tp, 1);
	}
	else if (tp->t_state & TS_XCLUDE && suser(td))
		return (EBUSY);

	/* XXX Discard return code */
	retval = ttyld_open(tp, dev);

	cons->kii.tty = tp;
	cons->kii.flags |= KII_DF_CONSOLE;
	cons->kii.priv.priv_ptr = cons;

	if (!sce_consoles[index]) {

		if ((err = kii_register_device(&cons->kii, index))) {

			KRN_ERROR("failed: could not register input (%d)", err);
			goto failed1;
		}

		/* Allocate a new render instance based on the render class
		 * registered to our display.
		 */
		if (!(cons->render = kgc_render_alloc(index, NULL))) {
			
			KRN_ERROR("failed: could not allocate render!");
			goto failed2;
		}
		((render_t) cons->render)->cons = cons;			/* XXX */
		if (RENDER_INIT((render_t)cons->render)) {
			
			KRN_ERROR("failed: could not init render!");
			goto failed3;
		}

		/* Allocate a new scroller instance based on the scroller class
		 * registered to our display.
		 */
		if (!(cons->scroller = kgc_scroller_alloc(index, NULL))) {
			
			KRN_ERROR("failed: could not allocate scroller!");
			goto failed4;
		}
		((render_t)cons->scroller)->cons = cons;		/* XXX */
		if (SCROLLER_INIT((scroller_t)cons->scroller, NULL)) {

			KRN_ERROR("failed: could not reset console");
			goto failed5;
		}

		/* Ok, we are now initialized */
		sce_consoles[index] = (sce_console_t *)cons;

		if (!kii_current_focus(cons->kii.focus_id))
			kii_map_device(cons->kii.id);

		KRN_DEBUG(4, "console %i allocated.", index);
	}

	/*
	 * The line discipline has clobbered t_winsize if TS_ISOPEN
	 * was clear. (NetBSD PR #400 from Bill Sommerfeld)
	 * We have to do this after calling the open routine, because
	 * it does some other things in other/older *BSD releases -hm
	 * See console_assign_parser().
	 */
	assign_parser(cons, (index)?1:0);

	return KGI_EOK;

 failed5:kgc_scroller_release(index);
 failed4:RENDER_DONE((render_t)cons->render);
 failed3:kgc_render_release(index);
 failed2:kii_unregister_device(&(cons->kii));
 failed1:
	if ((cons && index) || (cons && first_minor_allocated)) {
		kgi_kfree(cons);
		sce_consoles[index] = NULL;
		first_minor_allocated = 0;
	}

	s = spltty();
	if (tp) {
		ttyld_close(tp, flag);
		tty_close(tp);
	}
	splx(s);

	return (ENXIO);
}

static int
sce_vtclose(struct cdev *dev, int flag, int mode, struct thread *td)
{
	kgi_u_t index = minor(dev);
	kgi_console_t *cons;
	register struct tty *tp = dev->si_tty;
	int s;

	if (index >= CONFIG_KGII_MAX_NR_CONSOLES) {
		KRN_ERROR("Bad console!");
		return EINVAL;
	}

	cons = (kgi_console_t *)sce_consoles[index];

	if (cons && (index || first_minor_allocated)) {

		KRN_DEBUG(2, "freeing console %i", index);

		if (cons->kii.flags & KII_DF_FOCUSED)
			kii_unmap_device(cons->kii.id);

		kii_unregister_device(&cons->kii);

		SCROLLER_DONE((scroller_t)cons->scroller);
		cons->scroller = NULL;
		kgc_scroller_release(index);

		RENDER_DONE((render_t)cons->render);
		cons->render = NULL;
		kgc_render_release(index);

		sce_consoles[index] = NULL;
		kgi_kfree(cons);

		if (!index && first_minor_allocated)
			first_minor_allocated = 0;
	}

	s = spltty();
	ttyld_close(tp, flag);
	tty_close(tp);
	splx(s);

	return 0;
}

#if 0
const u_char *translate_special[2] = {
	{ NOP, K_VOID },		/* nothing (dead key)		*/
	{ LSH, K_NORMAL_SHIFTL },	/* left shift key		*/
	{ RSH, K_NORMAL_SHIFTR },	/* right shift key		*/
	{ CLK, K_CAPS },		/* caps lock key		*/
	{ NLK, K_NUM },			/* num lock key			*/
	{ SLK, K_PAUSE },		/* scroll lock key		*/
	{ LALT, K_NORMAL_ALT },		/* left alt key			*/
	{ BTAB, K_VOID },		/* backwards tab		*/
	{ LCTR, K_NORMAL_CTRLL },	/* left control key		*/
	{ NEXT, K_INCRCONSOLE },	/* switch to next screen 	*/
	{ F_SCR, K_VOID },		/* switch to first screen 	*/
	{ L_SCR, K_VOID },		/* switch to last screen 	*/
	{ F_FN, K_FIRST_FUNCTION },	/* first function key 		*/
	{ F(49), K_HOME },		/* cursor home			*/
	{ F(50), K_UP },		/* cursor up			*/
	{ F(51), K_PGUP },		/* cursor page up		*/
	{ F(57), K_END },		/* cursor end			*/
	{ F(58), K_DOWN },		/* cursor down			*/
	{ F(59), K_PGDN },		/* cursor page down		*/
	{ L_FN, K_VOID },		/* last function key 		*/
/*			0x7b-0x7f	   reserved do not use !	*/
	{ RCTR, K_NORMAL_CTRLR },	/* right control key		*/
	{ RALT, K_NORMAL_ALTGR },	/* right alt (altgr) key	*/
	{ ALK, K_LOCKED_ALT },		/* alt lock key			*/
	{ ASH, K_VOID },		/* alt shift key		*/
	{ META, K_VOID },		/* meta key			*/
	{ RBT, K_BOOT },		/* boot machine			*/
	{ DBG, K_SYSTEM_REQUEST },	/* call debugger		*/
	{ SUSP, K_SUSPEND },		/* suspend power (APM)		*/
	{ SPSC, K_TOGGLESCREEN },	/* toggle splash/text screen	*/

	{ DGRA, K_DGRAVE },		/* grave			*/
	{ DACU, K_DACUTE },		/* acute			*/
	{ DCIR, K_DCIRCM },		/* circumflex			*/
	{ DTIL, K_DTILDE },		/* tilde			*/
	{ DMAC, K_VOID },		/* macron			*/
	{ DBRE, K_VOID },		/* breve			*/
	{ DDOT, K_VOID },		/* dot				*/
	{ DUML, K_VOID },		/* umlaut/diaresis		*/
	{ DDIA, K_DDIERE },		/* diaresis			*/
	{ DSLA, K_VOID },		/* slash			*/
	{ DRIN, K_VOID },		/* ring				*/
	{ DCED, K_DCEDIL },		/* cedilla			*/
	{ DAPO, K_VOID },		/* apostrophe			*/
	{ DDAC, K_VOID },		/* double acute			*/
	{ DOGO, K_VOID },		/* ogonek			*/
	{ DCAR, K_VOID },		/* caron			*/

	{ STBY, K_STANDBY },		/* Go into standby mode (apm)   */
	{ PREV, K_DECRCONSOLE },	/* switch to previous screen 	*/
	{ PNC, K_PANIC },		/* force system panic */
	{ LSHA, K_VOID },		/* left shift key / alt lock	*/
	{ RSHA, K_VOID },		/* right shift key / alt lock	*/
	{ LCTRA, K_VOID },		/* left ctrl key / alt lock	*/
	{ RCTRA, K_VOID },		/* right ctrl key / alt lock	*/
	{ LALTA, K_VOID },		/* left alt key / alt lock	*/
	{ RALTA, K_VOID },		/* right alt key / alt lock	*/

	{ HALT, K_HALT },		/* halt machine */
	{ PDWN, K_POWERDOWN },		/* halt machine and power down */
	{ PASTE, K_VOID },		/* paste from cut-paste buffer */

	{ 0, 0 }
};
#endif

static int
sce_vtioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag, struct thread *td)
{
	register struct tty *tp = dev->si_tty;
	int error;

	if((error = ttioctl(tp, cmd, data, flag)) != ENOIOCTL)
		return (error);

	switch (cmd) {
	case GIO_KEYMAP:	/* get keyboard translation table */
		/* Translate from KII to KBD format */
		break;
	case PIO_KEYMAP:	/* set keyboard translation table */
#ifndef KBD_DISABLE_KEYMAP_LOAD
#endif
		break;
	case GIO_KEYMAPENT:	/* get keyboard translation table entry */
		break;
	case PIO_KEYMAPENT:	/* set keyboard translation table entry */
#ifndef KBD_DISABLE_KEYMAP_LOAD
#endif
		break;
	case GIO_DEADKEYMAP:	/* get accent key translation table */
		break;
	case PIO_DEADKEYMAP:	/* set accent key translation table */
#ifndef KBD_DISABLE_KEYMAP_LOAD
#endif
		break;
	case GETFKEY:		/* get functionkey string */
		break;
	case SETFKEY:		/* set functionkey string */
#ifndef KBD_DISABLE_KEYMAP_LOAD
#endif
		break;

	default:
		break;
	}

	return (ENOTTY);
}

static struct cdevsw scevt_cdevsw = {
	.d_open =	sce_vtopen,
	.d_close =	sce_vtclose,
	.d_read =	ttyread,
	.d_write =	ttywrite,
	.d_ioctl =	sce_vtioctl,
	.d_poll =	ttypoll,
	.d_name =	"scevt",
	.d_flags =	D_TTY | D_NEEDGIANT,
	.d_kqfilter =	ttykqfilter,
	.d_version =	D_VERSION
};

static int
scevt_mod_event(module_t mod, int type, void *data)
{
	int i;

	switch (type) {
	case MOD_LOAD:
		for(i = 0; i < 24 /* XXX should be automatic at kbd plug */; i++)
		{
			make_dev(&scevt_cdevsw, i, UID_ROOT, GID_WHEEL,
				 0600, "ttyv%r", i);
		}

#ifndef SC_NO_SYSMOUSE
		sce_mouse_init();
		sce_sysmouse_init();
#endif
		return 0;

	case MOD_UNLOAD:
		return ENXIO;

	default:
		break;
	}

	return 0;
}

static moduledata_t scevt_mod = {
	"scevt",
	scevt_mod_event,
	NULL,
};

DECLARE_MODULE(scevt, scevt_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_VERSION(scevt, 1);
